#ifndef LOADER_H
#define LOADER_H

#include <cstdint>
#include <cstddef>  
#include <algorithm>
#include <cstddef>  
#include <algorithm>
#include "catalog.h"


#define MAX_ROW_LEN 2048

extern char DISK_ROOT[512];

void setDiskRoot(const char *path);

bool loadCSV(const char* tsvPath, const char* schemaPath, bool longitFija);


bool escribirEnSector(const char* nombre, const char* linea, size_t len, bool longitFija);
void extraerNombreBase(const char* path, char* nombre);

bool yaExisteRelacion(const char* schemaPath, const char* nombre);

int obtenerCilindroParaRelacion(const char* nombre);

#endif 

