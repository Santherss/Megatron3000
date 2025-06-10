#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include "loader.h"      // Carga CSV, escribe en sectores, guarda esquema
#include "query.h"       // Consultas

using namespace std;

void menu() {
    cout << "\n% MEGATRON3000\n";
    cout << "  Welcome to MEGATRON 3000!\n";
    cout << "______________________________________________\n";
    cout << "\n&  1. Execute simple query\n";
    cout << "&  2. Execute query with condition\n";
    cout << "&  3. Exit\n";
    cout << "%\n";
    cout << "Option: ";
}

void menuPrincipal() {
    int option;
    while (true) {
        menu();
        if (!(cin >> option)) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Entrada inválida. Intente de nuevo.\n";
            continue;
        }
        cin.ignore();

        switch (option) {
            case 1:
                std::cout << "[DEBUG] Ejecutando consulta simple...\n";

                handleSimpleSelect();
                break;
            case 2:
                handleConditionalSelect();
                break;
            case 3:
                cout << "Exiting...\n";
                return;
            default:
                cout << "Invalid option\n";
        }
    }
}

int main() {
    const char* schemaPath = "schema/schema.txt";
    setDiskRoot("/home/riki/Documents/BD_II/Avance/Disco/disco");

    char csvPath[256];
    char modo;
    bool longitudFija = false;

    // Permitir cargar múltiples archivos CSV antes de consultar
    while (true) {
        cout << "Enter path to CSV file (e.g., data/titanicG.csv or data/Housing.csv): ";
        cin.getline(csvPath, sizeof(csvPath));

        cout << "¿Establecer como longitud fija? (s/n): ";
        cin >> modo;
        cin.ignore();

        longitudFija = (modo == 's' || modo == 'S');

        if (!loadCSV(csvPath, schemaPath, longitudFija)) {
            write(STDERR_FILENO, "Error loading database\n", 24);
        } else {
            cout << "Archivo cargado exitosamente.\n";
        }

        cout << "¿Deseas cargar otro archivo? (s/n): ";
        char seguir;
        cin >> seguir;
        cin.ignore();

        if (seguir == 'n' || seguir == 'N') break;
    }

    // Inicializar sistema de consultas (lee esquema desde disco)
    init_query(schemaPath);
    cout << "\nSistema de consultas inicializado. Puedes ejecutar consultas ahora.\n";

    // Mostrar menú principal
    menuPrincipal();

    return 0;
}

