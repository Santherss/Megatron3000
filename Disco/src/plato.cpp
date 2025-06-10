#include "plato.h"
#include "cara.h"
#include <filesystem>

Plato::Plato(int id, const std::string& basePath, int numCaras, int numPistas, int numSectores, int tamanoSector)
    : id(id), basePath(basePath), numCaras(numCaras), numPistas(numPistas), numSectores(numSectores), tamanoSector(tamanoSector) {}

void Plato::crearEstructura() {
    std::string pathPlato = basePath + "/Plato" + std::to_string(id);
    std::filesystem::create_directory(pathPlato);

    for (int i = 0; i < numCaras; ++i) {
        Cara cara(i, pathPlato, numPistas, numSectores, tamanoSector);
        cara.crearEstructura();
    }
}

