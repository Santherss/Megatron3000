#include "loader.h"
#include "query.h"
#include "disk_constants.h"
#include "printer.h"
#include "catalog.h"
#include "bufferManager.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <cctype>


void handleSimpleSelect(BufferManager& buffer) {
    printf("%% SELECT QUERY (e.g. SELECT * FROM tabla):\n> ");
    char input[256];
    if (!fgets(input, sizeof(input), stdin)) return;
    input[strcspn(input, "\n")] = '\0';  // quitar salto

    char* selectPtr = strstr(input, "SELECT");
    char* fromPtr = strstr(input, "FROM");
    if (!selectPtr || !fromPtr || fromPtr <= selectPtr) {
        printf("Consulta no válida\n");
        return;
    }

    char camposStr[128] = {0}, tablaName[128] = {0};
    strncpy(camposStr, selectPtr + 6, fromPtr - (selectPtr + 6));
    strcpy(tablaName, fromPtr + 4);

    // Eliminar espacios
    auto limpiar = [](char* str) {
        char* p = str;
        while ((p = strchr(str, ' '))) memmove(p, p + 1, strlen(p));
    };
    limpiar(camposStr);
    limpiar(tablaName);

    // Obtener esquema
    RelSchema esquema;
    char tempBuffer[1024];
    if (!obtenerEsquema(tablaName, tempBuffer, sizeof(tempBuffer))) {
        printf("Tabla no encontrada en el catálogo\n");
        return;
    }
    if (!parsearLineaEsquema(tempBuffer, esquema)) {
        printf("Error al parsear el esquema\n");
        return;
    }

    bool esVariable = !esquema.longitudFija;
    char (*fieldNames)[MAX_COL_LEN] = esquema.campos;
    int numFields = esquema.numColumnas;

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
                char temp[MAX_FIELD_LEN];
                strncpy(temp, fieldNames[j], MAX_FIELD_LEN);
                temp[MAX_FIELD_LEN - 1] = '\0';
                toLower(temp);
                if (strcmp(campos[i], temp) == 0) {
                    fieldIndices[numIndices++] = j;
                    found = true;
                    break;
                }
            }
            if (!found) {
                printf("Campo no encontrado: %s\n", campos[i]);
                return;
            }
        }
    }

    // Mostrar cabeceras
    char headers[16][MAX_FIELD_LEN];
    for (int i = 0; i < numIndices; ++i)
        strncpy(headers[i], fieldNames[fieldIndices[i]], MAX_FIELD_LEN);
    printHeaders(headers, numIndices);

    int filas = 0;

    for (int pista = esquema.cilindroInicio; pista <= esquema.cilindroFin; ++pista) {
        for (int cara = 0; cara < MAX_CARAS; ++cara) {
            for (int plato = 0; plato < MAX_PLATOS; ++plato) {
                for (int sector = 0; sector < MAX_SECTORES; sector += SECTORS_PER_PAGE) {
                    IDPagina id = {plato, cara, pista, sector};
                    BufferFrame* frame = buffer.fixPage(id, false);
                    if (!frame) continue;

                    int headerSize = esVariable ? sizeof(CabeceraVariable) : sizeof(CabeceraFija);
                    int contentSize = PAGE_SIZE - headerSize;
                    char* contenido = frame->data + headerSize;

                    char tempContenido[PAGE_SIZE]; 
                    memcpy(tempContenido, contenido, contentSize);
                    tempContenido[contentSize] = '\0';  
                   
                    char* linea = strtok(tempContenido, "\n");
                    while (linea) {
                        if (linea[0] == '\0' || linea[0] == '%') {
                            linea = strtok(NULL, "\n");
                            continue;
                        }
                        char campos[MAX_FIELDS][MAX_FIELD_LEN];
                        int n = split(linea, '#', campos);
                        printRow(campos, numIndices, fieldIndices, false);
                        filas++;
                        linea = strtok(NULL, "\n");
                    }

                    buffer.unfixPage(id, false);
                }
            }
        }
    }

    printf("Total de filas: %d\n", filas);
}

