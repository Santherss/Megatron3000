#ifndef DISCOVIRTUAL_H
#define DISCOVIRTUAL_H

#include <string>

class DiscoVirtual {
private:
    std::string base;
    int pistas;
    int sectores;
    int TAMANO_SECTOR;

    bool existe(const std::string& path);
    int contarPlatos();
    void guardarConfiguracion();
    bool leerConfiguracion();
    void expandirDisco(int nuevosPlatos);
    void eliminarPlato(int numeroPlato);
    void reindexarPlatos();
    int calcularTamano();

public:
    DiscoVirtual(const std::string& rutaBase);
    void mostrarMenu();
};

#endif

