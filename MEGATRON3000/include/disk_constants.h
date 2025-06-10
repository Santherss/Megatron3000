#ifndef DISK_CONSTANTS_H
#define DISK_CONSTANTS_H

#include <cstdint>

#define MAX_PISTAS 10
#define MAX_PLATOS 2
#define MAX_CARAS 2
#define MAX_SECTORES 16 


#define SECTOR_SIZE 512
#define HOUSING_RECORD_SIZE 64
#define TITANIC_RECORD_SIZE 128
#define PAGE_SIZE 2048
#define SECTORS_PER_PAGE 4

struct CabeceraFija {
    uint16_t num_registros;
    uint16_t tam_registro;
    uint16_t total_slots;
    uint8_t bitmap[4];
};

struct CabeceraVariable {
    uint16_t num_registros;
    uint16_t offset_libre;      // Donde empieza el espacio libre para datos
    uint16_t offset_slots;      // Donde empieza la tabla de slots (crece hacia arriba)
    uint16_t slots_count;       // Número actual de slots
    // Los slots se almacenan en la página, no aquí fijo
};




#endif

