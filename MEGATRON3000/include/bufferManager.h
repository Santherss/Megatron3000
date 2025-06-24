// bufferManager.h
#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H
#include "disk_constants.h"
#include <unordered_map>
#include <list>
#include <string>
#include <cstring>

const size_t MAX_FRAMES = 3;   // Tamaño del buffer pool

struct IDPagina {
    int plato;
    int cara;
    int pista;
    int sector; // sector inicial de la pagina

    bool operator==(const IDPagina& other) const {
        return plato == other.plato && cara == other.cara &&
               pista == other.pista && sector == other.sector;
    }
};

namespace std {
    template<>
    struct hash<IDPagina> {
        std::size_t operator()(const IDPagina& id) const {
            return ((id.plato * 31 + id.cara) * 31 + id.pista) * 31 + id.sector;
        }
    };
}

struct BufferFrame {
    char data[PAGE_SIZE];
    bool dirty;
    int pinCount;
    IDPagina id;
    bool isWrite;
};

class BufferManager {
public:
    BufferManager();
    ~BufferManager();

    BufferFrame* fixPage(const IDPagina& id, bool exclusive);
    void unfixPage(const IDPagina& id, bool isDirty);
    void flushAll();
    void printBufferTable();

private:
    std::unordered_map<IDPagina, BufferFrame*> pageTable;
    std::list<IDPagina> lruList;
    std::unordered_map<IDPagina, std::list<IDPagina>::iterator> lruMap;

    bool loadPageFromDisk(const IDPagina& id, char* data);
    void writePageToDisk(const IDPagina& id, const char* data);
    void evictPageIfNeeded();

};

#endif

