#include "disco.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sys/stat.h>
#include <algorithm>
#include <vector>

DiscoVirtual::DiscoVirtual(const std::string& rutaBase) : base(rutaBase), pistas(0), sectores(0), TAMANO_SECTOR(500) {
    std::filesystem::create_directories(base);
}

bool DiscoVirtual::existe(const std::string& path) {
    struct stat info;
    return stat(path.c_str(), &info) == 0;
}

int DiscoVirtual::contarPlatos() {
    int count = 0;
    while (true) {
        std::string pathPlato = base + "/Plato" + std::to_string(count);
        if (!existe(pathPlato)) break;
        count++;
    }
    return count;
}

void DiscoVirtual::guardarConfiguracion() {
    std::ofstream file(base + "/config.txt");
    file << pistas << " " << sectores << " " << TAMANO_SECTOR;
    file.close();
}

bool DiscoVirtual::leerConfiguracion() {
    std::ifstream file(base + "/config.txt");
    if (!file.is_open()) return false;
    file >> pistas >> sectores >> TAMANO_SECTOR;
    file.close();
    return true;
}

void DiscoVirtual::expandirDisco(int nuevosPlatos) {
    const int caras = 2;
    int platosExistentes = contarPlatos();
    int totalPlatos = platosExistentes + nuevosPlatos;
    int inicio = (nuevosPlatos == 0) ? 0 : platosExistentes;

    for (int p = inicio; p < totalPlatos; ++p) {
        std::string pathPlato = base + "/Plato" + std::to_string(p);
        if (!existe(pathPlato)) mkdir(pathPlato.c_str(), 0777);

        for (int c = 0; c < caras; ++c) {
            std::string nombreCara = (c == 0) ? "CaraSuperior" : "CaraInferior";
            std::string pathCara = pathPlato + "/" + nombreCara;
            if (!existe(pathCara)) mkdir(pathCara.c_str(), 0777);

            for (int pi = 0; pi < pistas; ++pi) {
                std::string pathPista = pathCara + "/Pista" + std::to_string(pi);
                if (!existe(pathPista)) mkdir(pathPista.c_str(), 0777);

                for (int s = 0; s < sectores; ++s) {
                    std::string pathSector = pathPista + "/Sector" + std::to_string(s);
                    if (!existe(pathSector)) {
                        std::ofstream archivo(pathSector);
                        archivo << std::string(TAMANO_SECTOR, ' ');
                        archivo.close();
                    }
                }
            }
        }
    }
}

void DiscoVirtual::eliminarPlato(int numeroPlato) {
    std::string pathPlato = base + "/Plato" + std::to_string(numeroPlato);
    if (existe(pathPlato)) {
        std::filesystem::remove_all(pathPlato);
        std::cout << "Plato " << numeroPlato << " eliminado correctamente.\n";
    } else {
        std::cout << "Plato no encontrado.\n";
    }
}

void DiscoVirtual::reindexarPlatos() {
    std::vector<std::string> platos;

    for (const auto& entry : std::filesystem::directory_iterator(base)) {
        if (entry.is_directory()) {
            std::string nombre = entry.path().filename().string();
            if (nombre.rfind("Plato", 0) == 0) {
                platos.push_back(nombre);
            }
        }
    }

    std::sort(platos.begin(), platos.end(), [](const std::string& a, const std::string& b) {
        int na = std::stoi(a.substr(5));
        int nb = std::stoi(b.substr(5));
        return na < nb;
    });

    for (size_t i = 0; i < platos.size(); ++i) {
        std::string oldName = base + "/" + platos[i];
        std::string newName = base + "/Plato" + std::to_string(i);
        if (oldName != newName) {
            std::filesystem::rename(oldName, newName);
        }
    }

    std::cout << "Reindexación completa. Platos reorganizados.\n";
}

int DiscoVirtual::calcularTamano() {
    int platos = contarPlatos();
    int totalSectores = platos * 2 * pistas * sectores;
    return totalSectores * TAMANO_SECTOR;
}

void DiscoVirtual::mostrarMenu() {
    int opcion;
    do {
        std::cout << "\nDISCO VIRTUAL\n";
        std::cout << "--------------------------------------------------\n";
        std::cout << "1. Agregar platos\n";
        std::cout << "2. Modificar pistas y sectores en todos los platos\n";
        std::cout << "3. Eliminar plato\n";
        std::cout << "4. Mostrar capacidad del disco\n";
        std::cout << "0. Salir\n";
        std::cout << "--------------------------------------------------\n";

        std::cout << "Opción: ";
        std::cin >> opcion;

        if (opcion == 1) {
            int platosAgregar;
            std::cout << "Cantidad de platos a agregar: ";
            std::cin >> platosAgregar;

            if (!leerConfiguracion()) {
                std::cout << "Cantidad de pistas por cara: ";
                std::cin >> pistas;
                std::cout << "Cantidad de sectores por pista: ";
                std::cin >> sectores;
                std::cout << "Tamaño del sector: ";
                std::cin >> TAMANO_SECTOR;
                guardarConfiguracion();
            } else {
                std::cout << "Usando configuración existente: "
                          << pistas << " pistas, " << sectores << " sectores.\n";
            }

            expandirDisco(platosAgregar);
        }
        else if (opcion == 2) {
            if (!leerConfiguracion()) {
                std::cout << "No hay configuración previa.\n";
            } else {
                std::cout << "Configuración actual: " << pistas << " pistas, " << sectores << " sectores.\n";
            }

            std::cout << "Nueva cantidad de pistas por cara: ";
            std::cin >> pistas;
            std::cout << "Nueva cantidad de sectores por pista: ";
            std::cin >> sectores;

            guardarConfiguracion();
            expandirDisco(0); // Solo modificar estructura sin agregar platos nuevos
        }
        else if (opcion == 3) {
            int num;
            std::cout << "Número de plato a eliminar: ";
            std::cin >> num;
            eliminarPlato(num);
            reindexarPlatos();
        }
        else if (opcion == 4) {
            if (!leerConfiguracion()) {
                std::cout << "No hay configuración previa.\n";
            } else {
                std::cout << "Cantidad actual de platos: " << contarPlatos() << "\n";
                std::cout << "Pistas por cara: " << pistas << "\n";
                std::cout << "Sectores por pista: " << sectores << "\n";
                std::cout << "Tamaño total del disco: " << calcularTamano() << " bytes\n";
            }
        }

    } while (opcion != 0);
}

