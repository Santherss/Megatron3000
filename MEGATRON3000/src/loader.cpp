#include "loader.h"
#include "disk_constants.h"
#include "query.h"
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>
#include <dirent.h>
#include <cstdint>
int csvs_cargados = 0;

char DISK_ROOT[512]="";
void setDiskRoot(const char* path) {
    strncpy(DISK_ROOT, path, sizeof(DISK_ROOT) - 1);
    DISK_ROOT[sizeof(DISK_ROOT) - 1] = '\0';
}

int contarRelaciones(const char* schemaPath) {
    int count = 0;
    int fd = open(schemaPath, O_RDONLY);
    if (fd < 0) return 0;

    char buffer[4096];
    ssize_t bytesRead = read(fd, buffer, sizeof(buffer));
    close(fd);

    for (ssize_t i = 0; i < bytesRead; ++i) {
        if (buffer[i] == '\n') count++;
    }
    return count;
}

void obtenerRangoCilindros(int& inicio, int& fin) {
    int mitad = MAX_PISTAS / 2;
    if (csvs_cargados == 0) {
        inicio = 0;
        fin = mitad - 1;
    } else {
        inicio = mitad;
        fin = MAX_PISTAS - 1;
    }
}


bool is_integer(const char* s) {
    if (*s == '-' || *s == '+') s++;
    if (*s == '\0') return false;
    while (*s) {
        if (*s < '0' || *s > '9') return false;
        s++;
    }
    return true;
}

bool is_float(const char* s) {
    bool dot_found = false;
    if (*s == '-' || *s == '+') s++;
    if (*s == '\0') return false;
    while (*s) {
        if (*s == '.') {
            if (dot_found) return false;
            dot_found = true;
        } else if (*s < '0' || *s > '9') return false;
        s++;
    }
    return dot_found;
}

bool parseHeader(char* linea, char columnas[][MAX_COL_LEN], int* num_cols) {
    *num_cols = 0;
    char* token = strtok(linea, ",");
    while (token && *num_cols < MAX_COLUMNS) {
        strncpy(columnas[*num_cols], token, MAX_COL_LEN - 1);
        (*num_cols)++;
        token = strtok(NULL, ",");
    }
    return (*num_cols > 0);
}

void inferirTipos(char tipos[][8], char* valores[], int num_cols) {
    for (int j = 0; j < num_cols; ++j) {
        if (is_integer(valores[j])) {
            strcpy(tipos[j], "int");
        } else if (is_float(valores[j])) {
            strcpy(tipos[j], "float");
        } else {
            strcpy(tipos[j], "string");
        }
    }
}

void guardarEsquema(int out_schema, const char* nombre, char columnas[][MAX_COL_LEN], char tipos[][8], int num_cols, bool longitudFija, int cilindroInicio, int cilindroFin) {
    char buffer[32];

    write(out_schema, nombre, strlen(nombre));
    for (int j = 0; j < num_cols; ++j) {
        write(out_schema, "#", 1);
        write(out_schema, columnas[j], strlen(columnas[j]));
        write(out_schema, "#", 1);
        write(out_schema, tipos[j], strlen(tipos[j]));
    }

    if(longitudFija)
        write(out_schema, "#fijo", 5);
    else
        write(out_schema, "#variable", 9);

    // Agrega "#inicio"
    write(out_schema, "#", 1);
    int len = sprintf(buffer, "%d", cilindroInicio);
    write(out_schema, buffer, len);

    // Agrega "#fin"
    write(out_schema, "#", 1);
    len = sprintf(buffer, "%d", cilindroFin);
    write(out_schema, buffer, len);

    write(out_schema, "\n", 1);
}


void extraerNombreBase(const char* path, char* nombre) {
    const char* p = strrchr(path, '/');
    p = p ? p + 1 : path;
    strncpy(nombre, p, 127);
    nombre[127] = '\0';
    char* punto = strrchr(nombre, '.');
    if (punto) *punto = '\0';
}

bool yaExisteRelacion(const char* schemaPath, const char* nombre) {
    int fd = open(schemaPath, O_RDONLY);
    if (fd < 0) return false;
    char buffer[4096] = {0};
    ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);
    return (bytes > 0 && strstr(buffer, nombre));
}

int parseCSVFields(char* line, char* values[], int max_cols) {
    int count = 0;
    bool inside_quotes = false;
    char* start = line;
    for (char* p = line; *p; ++p) {
        if (*p == '"') {
            inside_quotes = !inside_quotes;
        } else if (*p == ',' && !inside_quotes) {
            *p = '\0';
            if (count < max_cols) {
                values[count++] = start;
                start = p + 1;
            }
        }
    }
    if (count < max_cols) {
        values[count++] = start;
    }
    for (int i = 0; i < count; ++i) {
        char* val = values[i];
        size_t len = strlen(val);
        if (len >= 2 && val[0] == '"' && val[len - 1] == '"') {
            val[len - 1] = '\0';
            values[i] = val + 1;
        }
    }
    return count;
}

bool escribirEnPaginaFija(int cilindroInicio, const char* buffer, uint16_t tam_registro) {

    for (int pista = cilindroInicio; pista < MAX_PISTAS; ++pista) {
        for (int plato = 0; plato < MAX_PLATOS; ++plato) {
            for (int cara = 0; cara < MAX_CARAS; ++cara) {
                for (int sector = 0; sector + SECTORS_PER_PAGE <= MAX_SECTORES; sector += SECTORS_PER_PAGE) {
                    char pagina[PAGE_SIZE] = {0};
                    bool paginaExiste = true;

                    for (int i = 0; i < SECTORS_PER_PAGE; ++i) {
                        char path[1024];
                        snprintf(path, sizeof(path), "%s/Plato%d/Cara%d/Pista%d/Sector%d",DISK_ROOT, plato, cara, pista, sector + i);
                        int fd = open(path, O_RDONLY);
                        if (fd < 0 || read(fd, pagina + i * SECTOR_SIZE, SECTOR_SIZE) != SECTOR_SIZE) {
                            paginaExiste = false;
                            if (fd >= 0) close(fd);
                            break;
                        }
                        close(fd);
                    }

                    if (!paginaExiste) {
                        memset(pagina, 0, PAGE_SIZE);
                    }

                    CabeceraFija* cab = (CabeceraFija*)pagina;

                    if (cab->tam_registro != tam_registro || cab->total_slots == 0) {
                        cab->tam_registro = tam_registro;
                        cab->num_registros = 0;
                        cab->total_slots = (PAGE_SIZE - sizeof(CabeceraFija)) / tam_registro;
                        memset(cab->bitmap, 0, sizeof(cab->bitmap));
                    }
                    cab->total_slots = std::min((size_t)((PAGE_SIZE - sizeof(CabeceraFija)) / tam_registro),sizeof(cab->bitmap) * 8);
 

                   

                    for (int i = 0; i < cab->total_slots; ++i) {
                        int byte = i / 8, bit = i % 8;
                        if (!(cab->bitmap[byte] & (1 << bit))) {
                            cab->bitmap[byte] |= (1 << bit);
                            cab->num_registros++;
                            memcpy(pagina + sizeof(CabeceraFija) + i * tam_registro, buffer, tam_registro);

                            // Escribir los 4 sectores directamente
                            for (int j = 0; j < SECTORS_PER_PAGE; ++j) {
                                char path[1024];
                                snprintf(path, sizeof(path), "%s/Plato%d/Cara%d/Pista%d/Sector%d",
                                         DISK_ROOT, plato, cara, pista, sector + j);
                                int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                                if (fd < 0 || write(fd, pagina + j * SECTOR_SIZE, SECTOR_SIZE) != SECTOR_SIZE) {
                                    if (fd >= 0) close(fd);
                                    return false;
                                }
                                close(fd);
                            }

                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}

bool escribirEnPaginaVariable(int cilindroInicio, const char* buffer, size_t len) {
    for (int pista = cilindroInicio; pista < MAX_PISTAS; ++pista) {
        for (int plato = 0; plato < MAX_PLATOS; ++plato) {
            for (int cara = 0; cara < MAX_CARAS; ++cara) {
                for (int sector = 0; sector + SECTORS_PER_PAGE <= MAX_SECTORES; sector += SECTORS_PER_PAGE) {
                    char pagina[PAGE_SIZE] = {0};
                    bool paginaExiste = true;

                    for (int i = 0; i < SECTORS_PER_PAGE; ++i) {
                        char path[1024];
                        snprintf(path, sizeof(path), "%s/Plato%d/Cara%d/Pista%d/Sector%d",
                                 DISK_ROOT, plato, cara, pista, sector + i);
                        int fd = open(path, O_RDONLY);
                        if (fd < 0 || read(fd, pagina + i * SECTOR_SIZE, SECTOR_SIZE) != SECTOR_SIZE) {
                            paginaExiste = false;
                            if (fd >= 0) close(fd);
                            break;
                        }
                        close(fd);
                    }

                    if (!paginaExiste) {
                        memset(pagina, 0, PAGE_SIZE);
                    }

                    CabeceraVariable* cab = (CabeceraVariable*)pagina;

                    if (cab->offset_libre == 0 || cab->offset_slots == 0 || cab->offset_slots > PAGE_SIZE ||
                        cab->offset_libre < sizeof(CabeceraVariable)) {
                        cab->num_registros = 0;
                        cab->slots_count = 0;
                        cab->offset_libre = sizeof(CabeceraVariable);
                        cab->offset_slots = PAGE_SIZE;
                    }

                    // Alinear a sector para evitar cortar registros
                    int sector_relativo = cab->offset_libre / SECTOR_SIZE;
                    int pos_en_sector = cab->offset_libre % SECTOR_SIZE;
                    int espacio_en_sector = SECTOR_SIZE - pos_en_sector;

                    if (espacio_en_sector < (int)len) {
                        // Mover al inicio del siguiente sector
                        cab->offset_libre = (sector_relativo + 1) * SECTOR_SIZE;
                        pos_en_sector = 0;
                        espacio_en_sector = SECTOR_SIZE;
                    }

                    // Verificar si el registro completo y el slot caben en la página
                    if (cab->offset_libre + len <= cab->offset_slots - sizeof(uint16_t)) {
                        memcpy(pagina + cab->offset_libre, buffer, len);

                        cab->offset_slots -= sizeof(uint16_t);
                        uint16_t* slot = (uint16_t*)(pagina + cab->offset_slots);
                        *slot = cab->offset_libre;

                        cab->offset_libre += len;
                        cab->num_registros++;
                        cab->slots_count++;

                        for (int j = 0; j < SECTORS_PER_PAGE; ++j) {
                            char path[1024];
                            snprintf(path, sizeof(path), "%s/Plato%d/Cara%d/Pista%d/Sector%d",
                                     DISK_ROOT, plato, cara, pista, sector + j);
                            int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                            if (fd < 0 || write(fd, pagina + j * SECTOR_SIZE, SECTOR_SIZE) != SECTOR_SIZE) {
                                if (fd >= 0) close(fd);
                                return false;
                            }
                            close(fd);
                        }

                        return true;
                    }
                }
            }
        }
    }

    return false;
}



bool escribirEnSector(const char* linea, size_t len, bool longitudFija, int inicio) {
    if(longitudFija) return escribirEnPaginaFija(inicio,linea,len);
    else return escribirEnPaginaVariable(inicio, linea, len);
}


bool loadCSV(const char* tsvPath, const char* schemaPath, bool longitudFija) {
    char nombre[128];
    extraerNombreBase(tsvPath, nombre);

    char buffer[4096];
    int fd = open(tsvPath, O_RDONLY);
    if (fd < 0) {
        write(2, "Error abriendo archivo TSV\n", 27);
        return false;
    }

    char residual[MAX_ROW_LEN] = {0};
    size_t residual_len = 0;
    char columnas[MAX_COLUMNS][MAX_COL_LEN];
    char tipos[MAX_COLUMNS][8];
    bool header_done = false, tipos_inferidos = false;
    int num_cols = 0;
    int cilindroInicio, cilindroFin;

    if(contarRelaciones(schemaPath)==1){
        csvs_cargados++;
    }
    obtenerRangoCilindros(cilindroInicio, cilindroFin);

    while (true) {
        ssize_t bytesRead = read(fd, buffer, sizeof(buffer));
        if (bytesRead <= 0) break;

        size_t i = 0, start = 0;
        while (i < (size_t)bytesRead) {
            if (buffer[i] == '\n') {
                char linea[MAX_ROW_LEN] = {0};
                size_t len = 0;
                if (residual_len > 0) {
                    memcpy(linea, residual, residual_len);
                    len = residual_len;
                    residual_len = 0;
                }
                memcpy(linea + len, buffer + start, i - start);
                linea[len + (i - start)] = '\0';

                if (!header_done) {
                    header_done = parseHeader(linea, columnas, &num_cols);
                } else {
                    char* valores[MAX_COLUMNS];
                    int col = parseCSVFields(linea, valores, MAX_COLUMNS);

                    if (!tipos_inferidos && col == num_cols) {
                        inferirTipos(tipos, valores, col);
                        tipos_inferidos = true;
                    }

                    char lineaSalida[MAX_ROW_LEN] = {0};
                    size_t pos = 0;
                    for (int j = 0; j < col; ++j) {
                        size_t val_len = strlen(valores[j]);
                        if (pos + val_len + 2 >= sizeof(lineaSalida)) break;
                        memcpy(lineaSalida + pos, valores[j], val_len);
                        pos += val_len;
                        if (j < col - 1) lineaSalida[pos++] = '#';
                    }

                    // longitudFija es parámetro que recibes en loadCSV
                    size_t tam_registro = 0;

                    if (longitudFija) {
                        if (strcmp(nombre, "Housing") == 0) {
                            tam_registro = HOUSING_RECORD_SIZE;
                        }else{
                            tam_registro = TITANIC_RECORD_SIZE;
                        }

                        // Rellenar línea hasta tam_registro
                        while (pos < tam_registro - 1) lineaSalida[pos++] = ' ';
                        lineaSalida[pos++] = '\n';

                        if (!escribirEnSector(lineaSalida, tam_registro, longitudFija, cilindroInicio)) {
                            write(2, "Error escribiendo registro fijo\n", 32);
                            close(fd);
                            return false;
                        }
                    } else {
                        // Registro variable
                        lineaSalida[pos++] = '\n';
                        if (pos > SECTOR_SIZE) {
                            write(2, "Registro excede tamaño de sector\n", 34);
                            close(fd);
                            return false;
                        }
                        if (!escribirEnSector(lineaSalida, pos, longitudFija, cilindroInicio)) {
                            write(2, "Error escribiendo registro variable\n", 36);
                            close(fd);
                            return false;
                        }
                    }
                }
                start = i + 1;
            }
            ++i;
        }
        if (start < (size_t)bytesRead) {
            residual_len = bytesRead - start;
            memcpy(residual, buffer + start, residual_len);
        }
    }

    close(fd);
    csvs_cargados++;
    if (!yaExisteRelacion(schemaPath, nombre)) {
        int out_schema = open(schemaPath, O_RDWR | O_CREAT | O_APPEND, 0644);
        if (out_schema < 0) {
            write(2, "Error abriendo esquema\n", 24);
            return false;
        }
        // Moverse directamente al final real del archivo para agregar la nueva relación
        lseek(out_schema, 0, SEEK_END);

                // Escribir nueva relación
        guardarEsquema(out_schema, nombre, columnas, tipos, num_cols, longitudFija, cilindroInicio, cilindroFin);
        close(out_schema);
    }


    return true;
}




