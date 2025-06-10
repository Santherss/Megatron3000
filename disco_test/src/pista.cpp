#include "pista.h"
#include "sector.h"
#include <sys/stat.h>

Pista::Pista(const std::string& path, int sectores, int tamanoSector)
    : path(path), sectores(sectores), tamanoSector(tamanoSector) {}

void Pista::crear() const {
    mkdir(path.c_str(), 0777);
    for (int s = 0; s < sectores; ++s) {
        Sector sector(path + "/Sector" + std::to_string(s), tamanoSector);
        sector.crear();
    }
}

