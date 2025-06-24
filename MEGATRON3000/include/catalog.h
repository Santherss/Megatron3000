#ifndef CATALOG_H
#define CATALOG_H

#include <stdbool.h>
const int MAX_RELATION_NAME = 128;
const int MAX_COLUMNS = 20;
const int MAX_COL_LEN = 64;

// Estructura que representa una relación en el catálogo
struct RelSchema {
    char nombre[MAX_RELATION_NAME];           // Nombre de la relación
    char campos[MAX_COLUMNS][MAX_COL_LEN];    // Nombres de columnas
    char tipos[MAX_COLUMNS][8];               // Tipos de columnas (int, float, string)
    bool longitudFija;                        // true si es de longitud fija
    int cilindroInicio, cilindroFin;          // Rango de cilindros
    int numColumnas;                          // Número de columnas
};

// Lee una línea del esquema para una relación dada
bool obtenerEsquema(const char* nombre, char* buffer, int maxLen);

// Parsea una línea del esquema y llena una estructura RelSchema
bool parsearLineaEsquema(const char* linea, RelSchema& esquema);

#endif

