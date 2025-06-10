#ifndef PRINTER_H
#define PRINTER_H

#include "query.h"
#include <cstdio>
#include <cstring>

// Imprime todas las columnas de todas las filas
void printTableAll(const char headers[][MAX_FIELD_LEN],
                   const char rows[][MAX_FIELDS][MAX_FIELD_LEN],
                   int totalRows, int totalCols);

// Imprime solo una columna
void printTableField(const char headers[][MAX_FIELD_LEN],
                     const char rows[][MAX_FIELDS][MAX_FIELD_LEN],
                     int totalRows, int totalCols, int fieldIndex);

// Imprime solo columnas seleccionadas (por índices)
void printTableSelected(const char headers[][MAX_FIELD_LEN],
                        const char rows[][MAX_FIELDS][MAX_FIELD_LEN],
                        int totalRows,
                        const int* selectedIndices,
                        int numSelected);

#endif

