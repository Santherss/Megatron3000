#include "cara.h"
#include "pista.h"
#include <sys/stat.h>

Cara::Cara(const std::string& path, int pistas, int sectores, int tamanoSector)
    : path(path), pistas(pistas), sectores(sectores), tamanoSector(tamanoSector) {}

void Cara::crear() const {
    mkdir(path.c_str(), 0777);
    for (int p = 0; p < pistas; ++p) {
        Pista pista(path + "/Pista" + std::to_string(p), sectores, tamanoSector);
        pista.crear();
    }
}

