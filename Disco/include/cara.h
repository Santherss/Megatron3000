#ifndef CARA_H
#define CARA_H
#include <string>

class Cara {
    int id;
    std::string basePath;
    int numPistas, numSectores, tamanoSector;
public:
    Cara(int id, const std::string& basePath, int numPistas, int numSectores, int tamanoSector);
    void crearEstructura();
};

#endif
