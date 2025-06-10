#include "loader.h"
#include "query.h"
#include "disk_constants.h"
#include "printer.h"
#include <cstdio>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <ctype.h>
#include <cstdint>

bool leerSector(int plato, int cara, int pista, int sector, char* buffer) {
    if (plato < 0 || plato >= MAX_PLATOS ||
        cara < 0 || cara >= MAX_CARAS ||
        pista < 0 || pista >= MAX_PISTAS ||
        sector < 0 || sector >= MAX_SECTORES) {
        fprintf(stderr, "Parámetros fuera de rango\n");
        return false;
    }

    char path[1024];
    snprintf(path, sizeof(path), "%s/Plato%d/Cara%d/Pista%d/Sector%d",
             DISK_ROOT, plato, cara, pista, sector);
    
    printf("Intentando abrir el archivo: %s\n", path);
    FILE* f = fopen(path, "rb");
    if (!f) {
        perror("Error al abrir el archivo");
        return false;
    }

    size_t leidos = fread(buffer, 1, SECTOR_SIZE, f);
    fclose(f);

    return leidos == SECTOR_SIZE;
}

bool leerPagina(int plato, int cara, int pistaBase, int paginaRelativa, char* bufferPagina) {
    for (int i = 0; i < SECTORS_PER_PAGE; ++i) {
        int sector = paginaRelativa * SECTORS_PER_PAGE + i;
        if (sector >= 16) return false;
        if (!leerSector(plato, cara, pistaBase, sector, bufferPagina + i * SECTOR_SIZE))
            return false;
    }
    return true;
}

void trimStr(char* str) {
    char* start = str;
    while (*start && isspace(*start)) ++start;

    char* end = start + strlen(start) - 1;
    while (end > start && isspace(*end)) --end;
    *(end + 1) = '\0';

    if (start != str) memmove(str, start, strlen(start) + 1);
}

void handleSimpleSelect() {
    std::cout << "[debug] entrando a ejecutarConsultaSimple()\n";

    printf("%% SELECT QUERY (e.g. SELECT * FROM titanicG):\n> ");
    char input[256];
    if (!fgets(input, sizeof(input), stdin)) return;

    size_t len = strlen(input);
    if (len && input[len - 1] == '\n') input[len - 1] = '\0';

    char* selectPtr = strstr(input, "SELECT");
    char* fromPtr = strstr(input, "FROM");
    if (!selectPtr || !fromPtr || fromPtr <= selectPtr) {
        printf("consulta no válida\n");
        return;
    }

    char camposStr[128] = {0}, tablaName[128] = {0};
    strncpy(camposStr, selectPtr + 6, fromPtr - (selectPtr + 6));
    strcpy(tablaName, fromPtr + 4);

    char* p;
    while ((p = strchr(camposStr, ' '))) memmove(p, p + 1, strlen(p));
    while ((p = strchr(tablaName, ' '))) memmove(p, p + 1, strlen(p));
    toLower(tablaName);

    char buffer[SECTOR_SIZE];
    char fieldNames[MAX_FIELDS][MAX_FIELD_LEN];
    int numFields = 0;
    char tipoTabla[16] = {0};

    FILE* f = fopen(SCHEMA_PATH, "r");
    if (!f) {
        perror("Error al abrir schema.txt");
        return;
    }

    bool encontrado = false;
    while (fgets(buffer, sizeof(buffer), f)) {
        char nombreTablaArchivo[128];
        strncpy(nombreTablaArchivo, buffer, sizeof(nombreTablaArchivo));
        char* hash = strchr(nombreTablaArchivo, '#');
        if (hash) *hash = '\0';
        toLower(nombreTablaArchivo);

        if (strcmp(nombreTablaArchivo, tablaName) != 0)
            continue;

        // Línea encontrada
        encontrado = true;
        char* token = strtok(buffer, "#");
        int fIndex = 0;
        int tokenIndex = 0;
        char* ultimoToken = NULL;

        while ((token = strtok(NULL, "#")) && fIndex < MAX_FIELDS) {
            trimStr(token);
            ultimoToken = token;
            if (tokenIndex % 2 == 0) {
                strncpy(fieldNames[fIndex++], token, MAX_FIELD_LEN);
            }
            ++tokenIndex;
        }

        if (fIndex < 1 || !ultimoToken) {
            printf("esquema malformado para tabla %s\n", tablaName);
            fclose(f);
            return;
        }

        numFields = fIndex;
        strncpy(tipoTabla, ultimoToken, sizeof(tipoTabla));
        break;
    }
    fclose(f);

    if (!encontrado) {
        printf("tabla no encontrada en esquema\n");
        return;
    }

    bool showAll = (strcmp(camposStr, "*") == 0);
    int fieldIndices[MAX_FIELDS], numIndices = 0;

    if (showAll) {
        for (int i = 0; i < numFields; ++i) fieldIndices[numIndices++] = i;
    } else {
        char campos[MAX_FIELDS][MAX_FIELD_LEN];
        int n = split(camposStr, ',', campos);
        for (int i = 0; i < n; ++i) {
            toLower(campos[i]);
            bool found = false;
            for (int j = 0; j < numFields; ++j) {
                char tempField[MAX_FIELD_LEN];
                strncpy(tempField, fieldNames[j], MAX_FIELD_LEN);
                tempField[MAX_FIELD_LEN - 1] = '\0';
                toLower(tempField);
                if (strcmp(campos[i], tempField) == 0) {
                    fieldIndices[numIndices++] = j;
                    found = true;
                    break;
                }
            }
            if (!found) {
                printf("campo no encontrado: %s\n", campos[i]);
                return;
            }
        }
    }

    int startCilindro = 0, endCilindro = 4;
    if (strstr(tablaName, "housing")) {
        startCilindro = 5;
        endCilindro = 8;
    }

    char bufferPagina[PAGE_SIZE];
    char rows[MAX_ROWS][MAX_FIELDS][MAX_FIELD_LEN];
    int numRows = 0;

    int currentPlato = 0;
    int currentCara = 0;

    for (int currentPista = startCilindro; currentPista <= endCilindro; ++currentPista) {
        int numPaginas = 64;
        for (int pagina = 0; pagina < numPaginas && numRows < MAX_ROWS; ++pagina) {

            if (!leerPagina(currentPlato, currentCara, currentPista, pagina, bufferPagina)) continue;

            int headerSize = (strcmp(tipoTabla, "variable") == 0)
                             ? sizeof(CabeceraVariable)
                             : sizeof(CabeceraFija);
            char* registros = bufferPagina + headerSize;
            int i = 0, campo = 0, pos = 0;

            while (i < PAGE_SIZE - headerSize && numRows < MAX_ROWS) {
                if (registros[i] == '\0' || registros[i] == '\n') {
                    rows[numRows][campo][pos] = '\0';
                    numRows++;
                    campo = 0;
                    pos = 0;
                    ++i;
                    continue;
                } else if (registros[i] == '#') {
                    rows[numRows][campo][pos] = '\0';
                    ++campo;
                    pos = 0;
                } else {
                    if (pos < MAX_FIELD_LEN - 1)
                        rows[numRows][campo][pos++] = registros[i];
                }
                ++i;
            }
        }
    }

    if (numIndices == 1)
        printTableField(fieldNames, rows, numRows, numFields, fieldIndices[0]);
    else if (showAll)
        printTableAll(fieldNames, rows, numRows, numFields);
    else
        printTableSelected(fieldNames, rows, numRows, fieldIndices, numIndices);
}


