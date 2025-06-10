#include "plato.h"
#include "cara.h"
#include <sys/stat.h>

Plato::Plato(const std::string& path, int pistas, int sectores, int tamanoSector)
    : path(path), pistas(pistas), sectores(sectores), tamanoSector(tamanoSector) {}

void Plato::crear() const {
    mkdir(path.c_str(), 0777);
    Cara superior(path + "/CaraSuperior", pistas, sectores, tamanoSector);
    Cara inferior(path + "/CaraInferior", pistas, sectores, tamanoSector);
    superior.crear();
    inferior.crear();
}

