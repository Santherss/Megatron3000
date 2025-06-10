#ifndef PLATO_H
#define PLATO_H

#include <string>

class Plato {
    std::string path;
    int pistas;
    int sectores;
    int tamanoSector;

public:
    Plato(const std::string& path, int pistas, int sectores, int tamanoSector);
    void crear() const;
};

#endif

