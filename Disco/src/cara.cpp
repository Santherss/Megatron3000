#include "cara.h"
#include "pista.h"
#include <filesystem>

Cara::Cara(int id, const std::string& basePath, int numPistas, int numSectores, int tamanoSector)
    : id(id), basePath(basePath), numPistas(numPistas), numSectores(numSectores), tamanoSector(tamanoSector) {}

void Cara::crearEstructura() {
    std::string pathCara = basePath + "/Cara" + std::to_string(id);
    std::filesystem::create_directory(pathCara);

    for (int i = 0; i < numPistas; ++i) {
        Pista pista(i, pathCara, numSectores, tamanoSector);
        pista.crearEstructura();
    }
}

