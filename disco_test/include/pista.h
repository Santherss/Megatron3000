#ifndef PISTA_H
#define PISTA_H

#include <string>

class Pista {
    std::string path;
    int sectores;
    int tamanoSector;

public:
    Pista(const std::string& path, int sectores, int tamanoSector);
    void crear() const;
};

#endif

