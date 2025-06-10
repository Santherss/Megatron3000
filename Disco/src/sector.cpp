#include "sector.h"
#include <fstream>

Sector::Sector(int id, const std::string& basePath, int tamano)
    : id(id), basePath(basePath), tamano(tamano) {}

void Sector::crearArchivo() {
    std::string pathSector = basePath + "/Sector" + std::to_string(id);
    std::ofstream f(pathSector, std::ios::out);

    if (f) {
        char buffer = 0;
        for (int i = 0; i < tamano; ++i) {
            f<<' ';
        }
        f.close();
    }
}

