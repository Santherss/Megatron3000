#ifndef SECTOR_H
#define SECTOR_H

#include <string>

class Sector {
    std::string path;
    int tamano;

public:
    Sector(const std::string& path, int tamano);
    void crear() const;
};

#endif

