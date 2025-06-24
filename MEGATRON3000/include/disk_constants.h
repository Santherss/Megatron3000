#ifndef DISK_CONSTANTS_H
#define DISK_CONSTANTS_H

#include <cstdint>
constexpr int MAX_PISTAS = 10;
constexpr int MAX_PLATOS = 2;
constexpr int MAX_CARAS = 2;
constexpr int MAX_SECTORES = 16;

constexpr int SECTOR_SIZE = 512;
constexpr int PAGE_SIZE = 2048;
constexpr int SECTORS_PER_PAGE = 4;

constexpr int HOUSING_RECORD_SIZE = 64;
constexpr int TITANIC_RECORD_SIZE = 128;


struct CabeceraFija {
    uint16_t num_registros;
    uint16_t tam_registro;
    uint16_t total_slots;
    uint8_t bitmap[4];
};

struct CabeceraVariable {
    uint16_t num_registros;
    uint16_t offset_libre;          
    uint16_t offset_slots;          
    uint16_t slots_count;       // Número actual de slots
    };


#endif

