#include "sector.h"
#include <fstream>

Sector::Sector(const std::string& _path, int _tamano) : path(_path), tamano(_tamano) {}

void Sector::crear() const {
    std::ofstream archivo(path);
    archivo << std::string(tamano, ' ');
    archivo.close();
}

