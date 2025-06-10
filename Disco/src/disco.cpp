#include "disco.h"
#include "plato.h"
#include <filesystem>
#include <fstream>
#include <iostream>

Disco::Disco(const std::string& ruta) : rutaBase(ruta) {}

void Disco::inicializar() {
    std::filesystem::create_directories(rutaBase);

    for (int i = 0; i < NUM_PLATOS; ++i) {
        Plato plato(i, rutaBase, NUM_CARAS, NUM_PISTAS, NUM_SECTORES, TAMANO_SECTOR);
        plato.crearEstructura();
    }

    crearHeader();
    crearFreemap();
    crearIndex();
}

void Disco::crearHeader() {
    std::ofstream f(rutaBase + "/header.txt");
    f << "Platos: " << NUM_PLATOS << "\n";
    f << "Caras por plato: " << NUM_CARAS << "\n";
    f << "Pistas por cara: " << NUM_PISTAS << "\n";
    f << "Sectores por pista: " << NUM_SECTORES << "\n";
    f << "Tamaño sector: " << TAMANO_SECTOR << "\n";
    f.close();
}

void Disco::crearFreemap() {
    std::ofstream f(rutaBase + "/freemap.txt");
    int totalSectores = NUM_PLATOS * NUM_CARAS * NUM_PISTAS * NUM_SECTORES;
    for (int i = 0; i < totalSectores; ++i)
        f << "0\n"; // 0 = libre
    f.close();
}

void Disco::crearIndex() {
    std::ofstream f(rutaBase + "/index.txt");
    f << "#id\tsector_path\n";
    f.close();
}

