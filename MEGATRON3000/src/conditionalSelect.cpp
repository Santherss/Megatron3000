#include "loader.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include "disk_constants.h"
#include "query.h"
#include "printer.h"
#include <algorithm>


extern char g_schemaPath[512];

bool leerSectorFisico(int plato, int cara, int pista, int sector, char* contenido) {
    constexpr size_t MAX_PATH_LEN = 1024;
    char path[MAX_PATH_LEN];
    sprintf(path, "%s/Plato%d/Cara%d/Pista%d/Sector%d.bin", DISK_ROOT, plato, cara, pista, sector);
    FILE* f = fopen(path, "rb");
    if (!f) return false;
    fread(contenido, 1, SECTOR_SIZE, f);
    fclose(f);
    return true;
}

bool lecturaDePagina(int cilindro, int sectorInicial, char* pagina) {
    int plato = cilindro / 4;
    int cara  = (cilindro / 2) % 2;
    int pista = cilindro % 2;
    for (int i = 0; i < SECTORS_PER_PAGE; ++i) {
        if (!leerSectorFisico(plato, cara, pista, sectorInicial + i, pagina + i * SECTOR_SIZE))
            return false;
    }
    return true;
}

void trim(char* str) {
    int len = strlen(str), i = 0;
    while (isspace(str[i])) i++;
    if (i > 0) memmove(str, str + i, len - i + 1);
    len = strlen(str);
    while (len > 0 && isspace(str[len - 1])) str[--len] = '\0';
}

void handleConditionalSelect() {
    printf("%% Introduce consulta:\n> ");
    char input[512];
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;

    char* p = input;
    int j = 0;
    while (*p) {
        if (*p != '&' && *p != '#') input[j++] = *p;
        p++;
    }
    input[j] = '\0';
    trim(input);

    char* pipe = strchr(input, '|');
    char nombreArchivo[64] = "";
    if (pipe) {
        strcpy(nombreArchivo, pipe + 1);
        trim(nombreArchivo);
        *pipe = '\0';
        trim(input);
    }

    char selectCampos[128], tabla[64], campoCond[64], op[3], valorCond[64];
    if (sscanf(input, "SELECT %127[^F] FROM %63s WHERE %63[^<>=! ] %2[=<>!] \"%63[^\"]",
               selectCampos, tabla, campoCond, op, valorCond) != 5) {
        printf("Consulta mal formada.\n");
        return;
    }

    char camposSelect[16][64];
    int numCamposSelect = 0;
    int selectAll = strcmp(selectCampos, "*") == 0;
    if (!selectAll) {
        char* token = strtok(selectCampos, ",");
        while (token && numCamposSelect < 16) {
            trim(token);
            strcpy(camposSelect[numCamposSelect++], token);
            token = strtok(NULL, ",");
        }
    }

    // Leer esquema
    char nombres[64][128], tipos[64][16];
    int totalCampos = 0;
    for (int s = 0; s < 64; ++s) {
        char buffer[SECTOR_SIZE + 1] = {0};
        if (!leerSectorFisico(2, 0, 1, s, buffer)) break;
        char* linea = strtok(buffer, "\n");
        while (linea) {
            char* hash = strchr(linea, '#');
            if (!hash) { linea = strtok(NULL, "\n"); continue; }

            *hash = '\0';
            if (strcasecmp(linea, tabla) == 0) {
                char* campo = hash + 1;
                while (campo && totalCampos < 64) {
                    char* siguiente = strchr(campo, '#');
                    if (!siguiente) break;
                    *siguiente = '\0';
                    strcpy(nombres[totalCampos], campo);
                    campo = siguiente + 1;

                    siguiente = strchr(campo, '#');
                    if (!siguiente) break;
                    *siguiente = '\0';
                    strcpy(tipos[totalCampos], campo);
                    campo = siguiente + 1;

                    totalCampos++;
                }
                goto esquemaListo;
            }
            linea = strtok(NULL, "\n");
        }
    }
    printf("Tabla no encontrada en esquema\n");
    return;

esquemaListo:
    int idxCond = -1;
    int indicesSeleccionados[16], idxSel = 0;
    char nombresSeleccionados[16][128];

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
        sprintf(ruta, "data/%s.txt", nombreArchivo);
        fout = fopen(ruta, "w");
        if (!fout) {
            perror("No se pudo abrir archivo de salida");
            return;
        }
    }

    int totalFilas = 0;
    char filas[4096][64][128];  // filas seleccionadas

    char pagina[PAGE_SIZE + 1];
    for (int cil = 0; cil <= 8; ++cil) {
        for (int sec = 0; sec <= MAX_SECTORES - SECTORS_PER_PAGE; sec += SECTORS_PER_PAGE) {
            if (!lecturaDePagina(cil, sec, pagina)) continue;

            pagina[PAGE_SIZE] = '\0';
            char* linea = strtok(pagina, "\n");
            while (linea) {
                if (linea[0] == '\0' || linea[0] == '%') { linea = strtok(NULL, "\n"); continue; }

                char campos[64][128];
                int n = split(linea, '#', campos);
                if (idxCond >= n) { linea = strtok(NULL, "\n"); continue; }

                char err[64];
                if (evalCondition(campos[idxCond], op, valorCond, err)) {
                    if (fout) {
                        if (selectAll) {
                            for (int i = 0; i < n; ++i) {
                                fprintf(fout, "%s", campos[i]);
                                if (i < n - 1) fprintf(fout, "#");
                            }
                        } else {
                            for (int i = 0; i < idxSel; ++i) {
                                fprintf(fout, "%s", campos[indicesSeleccionados[i]]);
                                if (i < idxSel - 1) fprintf(fout, "#");
                            }
                        }
                        fprintf(fout, "\n");
                    }

                    // Guardar fila en array
                    for (int i = 0; i < (selectAll ? totalCampos : idxSel); ++i) {
                        int idx = selectAll ? i : indicesSeleccionados[i];
                        strcpy(filas[totalFilas][i], campos[idx]);
                    }

                    totalFilas++;
                }
                linea = strtok(NULL, "\n");
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

    printTableAll(
        selectAll ? nombres : nombresSeleccionados,
        filas,
        totalFilas,
        selectAll ? totalCampos : idxSel
    );
    printf("Total de filas seleccionadas: %d\n", totalFilas);
}

