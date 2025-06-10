#ifndef DISCO_H
#define DISCO_H
#include <string>
#include <vector>

class Disco {
public:
    static const int NUM_PLATOS = 2;
    static const int NUM_CARAS = 2;
    static const int NUM_PISTAS = 10;
    static const int NUM_SECTORES = 16;
    static const int TAMANO_SECTOR = 512;

    std::string rutaBase;

    Disco(const std::string& ruta);
    void inicializar();

private:
    void crearHeader();
    void crearFreemap();
    void crearIndex();
};

#endif
