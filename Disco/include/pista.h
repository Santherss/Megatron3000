#ifndef PISTA_H
#define PISTA_H
#include <string>

class Pista {
    int id;
    std::string basePath;
    int numSectores, tamanoSector;
public:
    Pista(int id, const std::string& basePath, int numSectores, int tamanoSector);
    void crearEstructura();
};

#endif
