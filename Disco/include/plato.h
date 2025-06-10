#ifndef PLATO_H
#define PLATO_H
#include <string>

class Plato {
    int id;
    std::string basePath;
    int numCaras, numPistas, numSectores, tamanoSector;
public:
    Plato(int id, const std::string& basePath, int numCaras, int numPistas, int numSectores, int tamanoSector);
    void crearEstructura();
};

#endif 
