/*#include "catalog.h"
#include "bufferManager.h"
#include "disk_constants.h"
#include "query.h"
#include <cstdio>
#include <cstring>
#include <cctype>

#define DEBUG_MODE 0

void handleUpdate(BufferManager& buffer) {
    printf("%% UPDATE QUERY (e.g. UPDATE tabla SET campo = \"nuevo\" WHERE campo = \"valor\"):\n> ");
    char input[512];
    if (!fgets(input, sizeof(input), stdin)) return;
    input[strcspn(input, "\n")] = 0;

    char tabla[64], campoTarget[64], nuevoValor[128], campoCond[64], valorCond[128];
    if (sscanf(input, "UPDATE %63s SET %63[^= ] = \"%127[^\"]\" WHERE %63[^= ] = \"%127[^\"]\"",
               tabla, campoTarget, nuevoValor, campoCond, valorCond) != 5) {
        printf("Consulta UPDATE no válida. Formato: UPDATE tabla SET campo = \"nuevo\" WHERE campo = \"valor\"\n");
        return;
    }

    char esquemaRaw[1024];
    RelSchema esquema;
    if (!obtenerEsquema(tabla, esquemaRaw, sizeof(esquemaRaw))) {
        printf("Tabla no encontrada en el catálogo.\n");
        return;
    }
    if (!parsearLineaEsquema(esquemaRaw, esquema)) {
        printf("Error al parsear el esquema.\n");
        return;
    }

    int idxCond = -1, idxTarget = -1;
    for (int i = 0; i < esquema.numColumnas; ++i) {
        if (strcasecmp(esquema.campos[i], campoCond) == 0) idxCond = i;
        if (strcasecmp(esquema.campos[i], campoTarget) == 0) idxTarget = i;
    }

    if (idxCond == -1 || idxTarget == -1) {
        printf("Campo no encontrado. Verifique los nombres.\n");
        return;
    }

    const bool esVariable = !esquema.longitudFija;
    int registrosModificados = 0;

    for (int pista = esquema.cilindroInicio; pista <= esquema.cilindroFin; ++pista) {
        for (int cara = 0; cara < MAX_CARAS; ++cara) {
            for (int plato = 0; plato < MAX_PLATOS; ++plato) {
                for (int sector = 0; sector < MAX_SECTORES; sector += SECTORS_PER_PAGE) {
                    IDPagina id = {plato, cara, pista, sector};
                    BufferFrame* frame = buffer.fixPage(id, true);
                    if (!frame) continue;

                    char* page = frame->data;
                    if (esPaginaVacia(page)) {
                        buffer.unfixPage(id, false);
                        continue;
                    }

                    bool modificada = false;

                    if (esVariable) {
                        CabeceraVariable* cab = reinterpret_cast<CabeceraVariable*>(page);
                        if (cab->offset_slots < sizeof(CabeceraVariable) ||
                            cab->offset_slots + cab->slots_count * sizeof(uint16_t) > PAGE_SIZE) {
                            buffer.unfixPage(id, false);
                            continue;
                        }

                        uint16_t* slots = reinterpret_cast<uint16_t*>(page + cab->offset_slots);
                        for (int i = 0; i < cab->slots_count; ++i) {
                            if (slots[i] >= PAGE_SIZE) continue;
                            char* registro = page + slots[i];
                            if (registro + 1 >= page + PAGE_SIZE) continue;

                            char copia[MAX_RECORD_LEN];
                            strncpy(copia, registro, MAX_RECORD_LEN - 1);
                            copia[MAX_RECORD_LEN - 1] = '\0';

                            char campos[MAX_FIELDS][MAX_FIELD_LEN];
                            int n = split(copia, '#', campos);
                            if (idxCond < n && strcmp(campos[idxCond], valorCond) == 0) {
                                strncpy(campos[idxTarget], nuevoValor, MAX_FIELD_LEN - 1);
                                campos[idxTarget][MAX_FIELD_LEN - 1] = '\0';

                                char nuevoRegistro[MAX_RECORD_LEN] = {0};
                                int len = join(campos, n, '#', nuevoRegistro, sizeof(nuevoRegistro));
                                if (len + slots[i] <= PAGE_SIZE) {
                                    strcpy(page + slots[i], nuevoRegistro);
                                    registrosModificados++;
                                    modificada = true;
                                }
                            }
                        }

                    } else {
                        CabeceraFija* cab = reinterpret_cast<CabeceraFija*>(page);
                        if (cab->tam_registro == 0 || cab->tam_registro > MAX_RECORD_LEN ||
                            cab->num_registros > 2048 / cab->tam_registro) {
                            buffer.unfixPage(id, false);
                            continue;
                        }

                        for (int i = 0; i < cab->num_registros; ++i) {
                            char* registro = page + sizeof(CabeceraFija) + i * cab->tam_registro;
                            if (registro + cab->tam_registro > page + PAGE_SIZE) break;

                            char copia[MAX_RECORD_LEN];
                            strncpy(copia, registro, cab->tam_registro);
                            copia[cab->tam_registro - 1] = '\0';

                            char campos[MAX_FIELDS][MAX_FIELD_LEN];
                            int n = split(copia, '#', campos);
                            if (idxCond < n && strcmp(campos[idxCond], valorCond) == 0) {
                                strncpy(campos[idxTarget], nuevoValor, MAX_FIELD_LEN - 1);
                                campos[idxTarget][MAX_FIELD_LEN - 1] = '\0';

                                char nuevoRegistro[MAX_RECORD_LEN] = {0};
                                join(campos, n, '#', nuevoRegistro, sizeof(nuevoRegistro));
                                strncpy(registro, nuevoRegistro, cab->tam_registro);
                                registrosModificados++;
                                modificada = true;
                            }
                        }
                    }

                    buffer.unfixPage(id, modificada);
                }
            }
        }
    }

    printf("Total de registros actualizados: %d\n", registrosModificados);
}
*/
