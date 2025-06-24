//#include "insert.h"
#include "query.h"
#include "bufferManager.h"
#include "catalog.h"
#include "loader.h"
#include <iostream>
#include <fstream>
#include <bitset>
#include <unistd.h>
#include <fcntl.h>
#include "disk_constants.h"

using namespace std;

void mostrarCabeceraPagina() {
    int plato, cara, pista, sector;
    char tipo;
    cout << "\nIngrese Plato [0-" << (MAX_PLATOS - 1) << "]: ";
    cin >> plato;
    cout << "Ingrese Cara [0-" << (MAX_CARAS - 1) << "]: ";
    cin >> cara;
    cout << "Ingrese Pista [0-" << (MAX_PISTAS - 1) << "]: ";
    cin >> pista;
    cout << "Ingrese Sector (inicio de página) [0-" << (MAX_SECTORES - SECTORS_PER_PAGE) << "]: ";
    cin >> sector;
    cout << "¿Es longitud fija (f) o variable (v)? ";
    cin >> tipo;

    string DISK_ROOT = "/Base_Datos/Disco/disco";
    char buffer[PAGE_SIZE];
    bool leido = true;

    for (int i = 0; i < SECTORS_PER_PAGE; ++i) {
        string path = DISK_ROOT + "/Plato" + to_string(plato) +
                      "/Cara" + to_string(cara) +
                      "/Pista" + to_string(pista) +
                      "/Sector" + to_string(sector + i);

        ifstream in(path, ios::binary);
        if (!in.is_open()) {
            cerr << "No se pudo abrir: " << path << "\n";
            leido = false;
            break;
        }
        in.read(buffer + i * SECTOR_SIZE, SECTOR_SIZE);
        in.close();
    }

    if (!leido) {
        cerr << "Error al leer los sectores.\n";
        return;
    }

    if (tipo == 'f' || tipo == 'F') {
        CabeceraFija* cab = reinterpret_cast<CabeceraFija*>(buffer);
        cout << "\n📄 Cabecera de Página de Longitud Fija\n";
        cout << "  - Número de registros   : " << cab->num_registros << "\n";
        cout << "  - Tamaño de registro    : " << cab->tam_registro << "\n";
        cout << "  - Total de slots bitmap : " << cab->total_slots << "\n";
        cout << "  - Bitmap (binario)      : ";
        for (int i = 0; i < 4; ++i)
            cout << bitset<8>(cab->bitmap[i]) << " ";
        cout << "\n";
    } else if (tipo == 'v' || tipo == 'V') {
        CabeceraVariable* cab = reinterpret_cast<CabeceraVariable*>(buffer);
        cout << "\n📄 Cabecera de Página de Longitud Variable\n";
        cout << "  - Número de registros   : " << cab->num_registros << "\n";
        cout << "  - Offset libre          : " << cab->offset_libre << "\n";
        cout << "  - Offset slots          : " << cab->offset_slots << "\n";
        cout << "  - Cantidad de slots     : " << cab->slots_count << "\n";
    } else {
        cerr << "Tipo desconocido.\n";
    }
}

void menu() {
    cout << "\n% MEGATRON3000\n";
    cout << "  Welcome to MEGATRON 3000!\n";
    cout << "______________________________________________\n";
    cout << "\n&  1. Execute simple query\n";
    cout << "&  2. Execute query with condition\n";
    cout << "&  3. Show buffer table\n";
    cout << "&  4. Insert records\n";
    cout << "&  5. Update records\n";
    cout << "&  6. Delete records\n";
    cout << "&  7. Inspect page header from disk\n";
    cout << "&  0. Exit\n";
    cout << "%\n";
    cout << "Option: ";
}

void menuPrincipal(BufferManager& globalBuffer) {
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
                handleSimpleSelect(globalBuffer);
                break;
            case 2:
                handleConditionalSelect(globalBuffer);
                break;
            case 3:
                globalBuffer.printBufferTable();
                break;
            case 4:
                //handleInsert(globalBuffer);
                //break;
            case 5:
                //handleUpdate(globalBuffer);
                //break;
            case 6:
            case 7:
                 mostrarCabeceraPagina();
                //break;
            case 0:
                cout << "Exiting...\n";
                return;
            default:
                cout << "Invalid option\n";
        }
    }
}

int main() {
    const char* schemaPath = "schema/schema.txt";
    setDiskRoot("../Disco/disco");
    BufferManager globalBuffer;
    char csvPath[256];
    char modo;
    bool longitudFija = false;

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

    init_query(schemaPath);
    cout << "\nSistema de consultas inicializado. Puedes ejecutar consultas ahora.\n";

    menuPrincipal(globalBuffer);
    return 0;
}

