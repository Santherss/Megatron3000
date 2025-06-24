/*#include "catalog.h"
#include "bufferManager.h"
#include "disk_constants.h"
#include "query.h"
#include <cstdio>
#include <cstring>
#include <cctype>

#define DEBUG_MODE 0

void handleInsert(BufferManager& buffer) {
    printf("%% INSERT QUERY (e.g. INSERT INTO tabla VALUES \"campo1#campo2#...\"):\n> ");
    char input[512];
    if (!fgets(input, sizeof(input), stdin)) return;
    input[strcspn(input, "\n")] = 0;

    char tabla[64], registro[MAX_RECORD_LEN];
    if (sscanf(input, "INSERT INTO %63s VALUES \"%127[^\"]\"", tabla, registro) != 2) {
        printf("Consulta INSERT no válida. Formato: INSERT INTO tabla VALUES \"campo1#campo2#...\"\n");
        return;
    }

    char bufferTemp[1024];
    RelSchema esquema;
    if (!obtenerEsquema(tabla, bufferTemp, sizeof(bufferTemp))) {
        printf("Tabla no encontrada en el catálogo\n");
        return;
    }
    if (!parsearLineaEsquema(bufferTemp, esquema)) {
        printf("Error al parsear el esquema\n");
        return;
    }

    const bool esVariable = !esquema.longitudFija;
    const int tamanoRegistro = esquema.longitudRegistro;
    bool insertado = false;

    for (int pista = esquema.cilindroInicio; pista <= esquema.cilindroFin && !insertado; ++pista) {
        for (int cara = 0; cara < MAX_CARAS && !insertado; ++cara) {
            for (int plato = 0; plato < MAX_PLATOS && !insertado; ++plato) {
                for (int sector = 0; sector < MAX_SECTORES && !insertado; sector += SECTORS_PER_PAGE) {
                    IDPagina id = {plato, cara, pista, sector};
                    BufferFrame* frame = buffer.fixPage(id, true);
                    if (!frame) continue;

                    char* page = frame->data;

                    if (esVariable) {
                        CabeceraVariable* cab = reinterpret_cast<CabeceraVariable*>(page);
                        if (cab->offset_slots == 0) {
                            cab->offset_slots = PAGE_SIZE;
                            cab->num_registros = 0;
                            cab->slots_count = 0;
                        }

                        int longitud = strlen(registro) + 1;
                        int offsetDisponible = cab->offset_slots - longitud;

                        if (offsetDisponible < sizeof(CabeceraVariable) + (cab->slots_count + 1) * sizeof(uint16_t)) {
                            buffer.unfixPage(id, false); // no hay espacio
                            continue;
                        }

                        // Copiar registro
                        memcpy(page + offsetDisponible, registro, longitud);
                        cab->offset_slots = offsetDisponible;

                        // Actualizar slot
                        uint16_t* slots = reinterpret_cast<uint16_t*>(page + cab->offset_slots + longitud);
                        slots[cab->slots_count++] = offsetDisponible;
                        cab->num_registros++;

                        insertado = true;
                        buffer.unfixPage(id, true);
                        break;
                    } else {
                        CabeceraFija* cab = reinterpret_cast<CabeceraFija*>(page);
                        if (cab->bitmap == 0) {
                            std::memset(page + sizeof(CabeceraFija), 0, PAGE_SIZE - sizeof(CabeceraFija));
                        }

                        int maxRegs = (PAGE_SIZE - sizeof(CabeceraFija)) / tamanoRegistro;
                        for (int i = 0; i < maxRegs; ++i) {
                            if (!(cab->bitmap & (1ULL << i))) {
                                char* dst = page + sizeof(CabeceraFija) + i * tamanoRegistro;
                                strncpy(dst, registro, tamanoRegistro - 1);
                                dst[tamanoRegistro - 1] = '\0';
                                cab->bitmap |= (1ULL << i);
                                cab->num_registros++;
                                insertado = true;
                                buffer.unfixPage(id, true);
                                break;
                            }
                        }

                        if (!insertado)
                            buffer.unfixPage(id, false);
                    }
                }
            }
        }
    }

    if (insertado)
        printf("Registro insertado correctamente.\n");
    else
        printf("No se pudo insertar el registro: páginas llenas.\n");
}
*/
