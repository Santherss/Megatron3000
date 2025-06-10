#ifndef CARA_H
#define CARA_H

#include <string>

class Cara {
    std::string path;
    int pistas;
    int sectores;
    int tamanoSector;

public:
    Cara(const std::string& path, int pistas, int sectores, int tamanoSector);
    void crear() const;
};

#endif

