#include "loader.h"
#include "catalog.h"
#include "query.h"
#include "bufferManager.h"
#include "printer.h"
#include "disk_constants.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

void handleConditionalSelect(BufferManager& buffer) {
    printf("%% Introduce consulta:\n> ");
    char input[512];
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;

    // Eliminar caracteres no deseados (&, #)
    char* p = input;
    int j = 0;
    while (*p) {
        if (*p != '&' && *p != '#') input[j++] = *p;
        p++;
    }
    input[j] = '\0';

    // Ver si hay nombre de archivo de salida (usando '|')
    char* pipe = strchr(input, '|');
    char nombreArchivo[64] = "";
    if (pipe) {
        strcpy(nombreArchivo, pipe + 1);
        while (isspace(nombreArchivo[0])) memmove(nombreArchivo, nombreArchivo + 1, strlen(nombreArchivo));
        *pipe = '\0';
    }

    // Parsear SELECT con WHERE
    char selectCampos[128], tabla[64], campoCond[64], op[3], valorCond[64];
    if (sscanf(input, "SELECT %127[^F] FROM %63s WHERE %63[^<>=! ] %2[=<>!] \"%63[^\"]",
               selectCampos, tabla, campoCond, op, valorCond) != 5) {
        printf("Consulta mal formada.\n");
        return;
    }

    // Obtener esquema del catálogo
    char tempBuffer[1024];
    RelSchema esquema;
    if (!obtenerEsquema(tabla, tempBuffer, sizeof(tempBuffer))) {
        printf("Tabla no encontrada en el catálogo\n");
        return;
    }
    if (!parsearLineaEsquema(tempBuffer, esquema)) {
        printf("Error al parsear el esquema\n");
        return;
    }

    const bool esVariable = !esquema.longitudFija;
    char (*nombres)[MAX_COL_LEN] = esquema.campos;
    char (*tipos)[8] = esquema.tipos;
    const int totalCampos = esquema.numColumnas;

    // Parsear SELECT
    char camposSelect[16][64];
    int numCamposSelect = 0;
    const bool selectAll = strcmp(selectCampos, "*") == 0;
    if (!selectAll) {
        char* token = strtok(selectCampos, ",");
        while (token && numCamposSelect < 16) {
            while (isspace(*token)) ++token;
            char* end = token + strlen(token) - 1;
            while (end > token && isspace(*end)) *end-- = '\0';
            strcpy(camposSelect[numCamposSelect++], token);
            token = strtok(NULL, ",");
        }
    }

    // Índices
    int idxCond = -1;
    int indicesSeleccionados[16], idxSel = 0;
    char nombresSeleccionados[16][MAX_COL_LEN];

    for (int i = 0; i < totalCampos; ++i) {
        if (strcasecmp(nombres[i], campoCond) == 0) idxCond = i;
        if (!selectAll) {
            for (int j = 0; j < numCamposSelect; ++j) {
                if (strcasecmp(nombres[i], camposSelect[j]) == 0) {
                    indicesSeleccionados[idxSel] = i;
                    strcpy(nombresSeleccionados[idxSel], nombres[i]);
                    idxSel++;
                    break;
                }
            }
        }
    }

    if (idxCond == -1) {
        printf("Campo de condición no encontrado.\n");
        return;
    }

    FILE* fout = NULL;
    if (nombreArchivo[0]) {
        char ruta[128];
        snprintf(ruta, sizeof(ruta), "data/%s.txt", nombreArchivo);
        fout = fopen(ruta, "w");
        if (!fout) {
            perror("No se pudo abrir archivo de salida");
            return;
        }
    }

    // Encabezados
    char headers[16][MAX_FIELD_LEN];
    int headerCount = selectAll ? totalCampos : idxSel;
    for (int i = 0; i < headerCount; ++i) {
        const char* src = selectAll ? nombres[i] : nombresSeleccionados[i];
        strncpy(headers[i], src, MAX_FIELD_LEN);
    }

    printHeaders(headers, headerCount);

    int filasEncontradas = 0;
    for (int pista = esquema.cilindroInicio; pista <= esquema.cilindroFin; ++pista) {
        for (int cara = 0; cara < MAX_CARAS; ++cara) {
            for (int plato = 0; plato < MAX_PLATOS; ++plato) {
                for (int sector = 0; sector < MAX_SECTORES; sector += SECTORS_PER_PAGE) {
                    IDPagina id = {plato, cara, pista, sector};
                    BufferFrame* frame = buffer.fixPage(id, false);
                    if (!frame) continue;

                    int headerSize = esVariable ? sizeof(CabeceraVariable) : sizeof(CabeceraFija);
                    char* contenido = frame->data + headerSize;
                    contenido[PAGE_SIZE - headerSize] = '\0';

                    char* linea = strtok(contenido, "\n");
                    while (linea) {
                        if (linea[0] == '\0' || linea[0] == '%') {
                            linea = strtok(NULL, "\n");
                            continue;
                        }
                        char campos[MAX_FIELDS][MAX_FIELD_LEN];
                        int n = split(linea, '#', campos);
                        if (idxCond >= n) {
                            linea = strtok(NULL, "\n");
                            continue;
                        }

                        char err[64];
                        if (evalCondition(campos[idxCond], op, valorCond, err)) {
                            if (fout) {
                                for (int i = 0; i < headerCount; ++i) {
                                    int idx = selectAll ? i : indicesSeleccionados[i];
                                    fprintf(fout, "%s", campos[idx]);
                                    if (i < headerCount - 1) fprintf(fout, "#");
                                }
                                fprintf(fout, "\n");
                            }
                            printRow(campos, headerCount, indicesSeleccionados, selectAll);
                            filasEncontradas++;
                        }
                        linea = strtok(NULL, "\n");
                    }
                    buffer.unfixPage(id, false);
                }
            }
        }
    }

    if (fout) {
        fclose(fout);
        FILE* schemaOut = fopen(g_schemaPath, "a");
        if (schemaOut) {
            fprintf(schemaOut, "%s", nombreArchivo);
            if (selectAll) {
                for (int i = 0; i < totalCampos; ++i)
                    fprintf(schemaOut, "#%s#%s", nombres[i], tipos[i]);
            } else {
                for (int i = 0; i < idxSel; ++i)
                    fprintf(schemaOut, "#%s#%s", nombresSeleccionados[i], tipos[indicesSeleccionados[i]]);
            }
            fprintf(schemaOut, "\n");
            fclose(schemaOut);
            printf("Resultados guardados: data/%s.txt\n", nombreArchivo);
        }
    }

    printf("Total de filas seleccionadas: %d\n", filasEncontradas);
}

