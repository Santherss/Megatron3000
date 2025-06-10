#include "pista.h"
#include "sector.h"
#include <filesystem>

Pista::Pista(int id, const std::string& basePath, int numSectores, int tamanoSector)
    : id(id), basePath(basePath), numSectores(numSectores), tamanoSector(tamanoSector) {}

void Pista::crearEstructura() {
    std::string pathPista = basePath + "/Pista" + std::to_string(id);
    std::filesystem::create_directory(pathPista);

    for (int i = 0; i < numSectores; ++i) {
        Sector sector(i, pathPista, tamanoSector);
        sector.crearArchivo();
    }
}

