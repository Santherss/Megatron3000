#ifndef QUERY_H
#define QUERY_H

#include <stddef.h>  // para size_t
#include <stdbool.h> // para bool

#define MAX_FIELDS 64
#define MAX_FIELD_LEN 128
#define MAX_RECORD_LEN 200
#define MAX_ROWS 1000
#define SCHEMA_PATH "schema/schema.txt"

extern char g_schemaPath[512];

// Inicializa el sistema de consultas
void init_query(const char* schemaPath);

// Divide un string por delimitador `delim`
// `output` será un arreglo de C-strings de salida con `MAX_FIELDS` entradas
// Retorna el número de partes extraídas
int split(const char* str, char delim, char output[MAX_FIELDS][MAX_FIELD_LEN]);

// Convierte una cadena a minúsculas (in-place)
void toLower(char* str);

// Evalúa una condición simple: valor de campo, operador, valor de condición
// `errorOutput` guarda mensaje de error si hay fallo
bool evalCondition(const char* fieldValue, const char* op, const char* condValue, char* errorOutput);

// Muestra el tamaño de un archivo (solo útil para debug)
void mostrarTamanoArchivo(const char* rutaArchivo);

// Ejecuta SELECT sin condiciones
void handleSimpleSelect();

// Ejecuta SELECT con condiciones
void handleConditionalSelect();

#endif

