#include "catalog.h"
#include "query.h"  // <- para acceder a g_schemaPath
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>

bool obtenerEsquema(const char* nombre, char* buffer, int maxLen) {
    int fd = open(g_schemaPath, O_RDONLY);
    if (fd < 0){
        std::cerr << "[error] No se pudo abrir el archivo de esquema: " << g_schemaPath << std::endl;
        return false;
    }     
    std::cout << "[debug] Buscando tabla '" << nombre << "' en '" << g_schemaPath << "'..." << std::endl;
 
    char fileBuf[4096];
    ssize_t bytesRead = read(fd, fileBuf, sizeof(fileBuf) - 1);
    close(fd);
    if (bytesRead <= 0) return false;

    fileBuf[bytesRead] = '\0';
    char* linea = strtok(fileBuf, "\n");
    while (linea) {
        if (strncmp(linea, nombre, strlen(nombre)) == 0 && linea[strlen(nombre)] == '#') {
            strncpy(buffer, linea, maxLen);
            buffer[maxLen - 1] = '\0';
            return true;
        }
        linea = strtok(nullptr, "\n");
    }
    return false;
}

bool parsearLineaEsquema(const char* linea, RelSchema& esquema) {
    char copia[1024];
    strncpy(copia, linea, sizeof(copia));
    copia[sizeof(copia) - 1] = '\0';

    char* token = strtok(copia, "#");
    if (!token) return false;
    strncpy(esquema.nombre, token, MAX_RELATION_NAME);

    int col = 0;
    while ((token = strtok(nullptr, "#")) && col < MAX_COLUMNS) {
        if (strcmp(token, "fijo") == 0 || strcmp(token, "variable") == 0) {
            esquema.longitudFija = (strcmp(token, "fijo") == 0);
            break;
        }
        strncpy(esquema.campos[col], token, MAX_COL_LEN);

        token = strtok(nullptr, "#");
        if (!token) return false;
        strncpy(esquema.tipos[col], token, 8);
        col++;
    }
    esquema.numColumnas = col;

    token = strtok(nullptr, "#");
    if (!token) return false;
    esquema.cilindroInicio = atoi(token);

    token = strtok(nullptr, "#");
    if (!token) return false;
    esquema.cilindroFin = atoi(token);

    return true;
}

