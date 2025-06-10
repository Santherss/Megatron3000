#ifndef SECTOR_H
#define SECTOR_H

#include <string>

class Sector {
    int id;
    std::string basePath;
    int tamano;
public:
    Sector(int id, const std::string& basePath, int tamano);
    void crearArchivo();
};
#endif
