#ifndef QUERY_H
#define QUERY_H

#include "bufferManager.h"
#include <stddef.h>  
#include <stdbool.h> 

#define MAX_FIELDS 20
#define MAX_FIELD_LEN 64
#define MAX_RECORD_LEN 200
#define MAX_ROWS 1000
#define SCHEMA_PATH "schema/schema.txt"

extern char g_schemaPath[512];

void init_query(const char* schemaPath);

// Divide un string por delimitador `delim`
int split(const char* str, char delim, char output[MAX_FIELDS][MAX_FIELD_LEN]);

// Convierte una cadena a minúsculas (in-place)
void toLower(char* str);

// Evalúa una condición simple: valor de campo, operador, valor de condición
// `errorOutput` guarda mensaje de error si hay fallo
bool evalCondition(const char* fieldValue, const char* op, const char* condValue, char* errorOutput);

// Muestra el tamaño de un archivo
void mostrarTamanoArchivo(const char* rutaArchivo);

// SELECT sin condiciones
void handleSimpleSelect(BufferManager& buffer);

// SELECT con condiciones
void handleConditionalSelect(BufferManager(& buffer));

#endif

