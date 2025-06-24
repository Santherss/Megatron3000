#include "bufferManager.h"
#include "loader.h"
#include <iomanip>
#include "disk_constants.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <cstring>
#include <string>
#include <sstream>

BufferManager::BufferManager() {}

BufferManager::~BufferManager() {
    flushAll();
    for (auto& pair : pageTable) {
        delete pair.second;
    }
}

BufferFrame* BufferManager::fixPage(const IDPagina& id, bool exclusive) {
    if (pageTable.count(id)) {
        BufferFrame* frame = pageTable[id];
        frame->pinCount++;
        frame->isWrite |= exclusive;

        if (lruMap.count(id)) {
            lruList.erase(lruMap[id]);
            lruList.push_front(id);
            lruMap[id] = lruList.begin();
        }
        return frame;
    }

    evictPageIfNeeded();

    BufferFrame* newFrame = new BufferFrame;
    newFrame->dirty = false;
    newFrame->pinCount = 1;
    newFrame->id = id;
    newFrame->isWrite = exclusive;

    if (!loadPageFromDisk(id, newFrame->data)) {
        delete newFrame;
        return nullptr;
    }

    pageTable[id] = newFrame;
    lruList.push_front(id);
    lruMap[id] = lruList.begin();

    return newFrame;
}

void BufferManager::unfixPage(const IDPagina& id, bool isDirty) {
    if (!pageTable.count(id)) return;

    BufferFrame* frame = pageTable[id];
    if (frame->pinCount > 0)
        frame->pinCount--;

    if (isDirty)
        frame->dirty = true;
}

void BufferManager::flushAll() {
    for (auto& pair : pageTable) {
        BufferFrame* frame = pair.second;
        
        if (frame->dirty && frame->isWrite) {
            writePageToDisk(frame->id, frame->data);
            frame->dirty = false;
        }
    }
}

void BufferManager::evictPageIfNeeded() {
    if (pageTable.size() < MAX_FRAMES) return;

    for (auto it = lruList.rbegin(); it != lruList.rend(); ++it) {
        IDPagina id = *it;
        BufferFrame* frame = pageTable[id];
        if (frame->pinCount == 0) {
            if (frame->dirty && frame->isWrite) {
                writePageToDisk(id, frame->data);
            }
            delete frame;
            pageTable.erase(id);
            lruList.erase(std::next(it).base());
            lruMap.erase(id);
            return;
        }
    }

    std::cerr << "No se pudo desalojar ninguna página: todas están fijadas.\n";
}

bool BufferManager::loadPageFromDisk(const IDPagina& id, char* data) {
    bool algunSectorExiste = false;

    for (int i = 0; i < SECTORS_PER_PAGE; ++i) {
        std::string path = std::string(DISK_ROOT) + "/Plato" + std::to_string(id.plato) +
                           "/Cara" + std::to_string(id.cara) +
                           "/Pista" + std::to_string(id.pista) +
                           "/Sector" + std::to_string(id.sector + i);

        std::ifstream in(path, std::ios::binary);
        if (in.is_open()) {
            in.read(data + i * SECTOR_SIZE, SECTOR_SIZE);
            in.close();
            algunSectorExiste = true;
        } else {
            std::memset(data + i * SECTOR_SIZE, 0, SECTOR_SIZE);
        }
    }

    bool todoCero = true;
    for (int i = 0; i < PAGE_SIZE; ++i) {
        if (data[i] != 0) {
            todoCero = false;
            break;
        }
    }

    return algunSectorExiste && !todoCero;
}

void BufferManager::writePageToDisk(const IDPagina& id, const char* data) {
    for (int i = 0; i < SECTORS_PER_PAGE; ++i) {
        std::string path = std::string(DISK_ROOT) + "/Plato" + std::to_string(id.plato) +
                           "/Cara" + std::to_string(id.cara) +
                           "/Pista" + std::to_string(id.pista) +
                           "/Sector" + std::to_string(id.sector + i);

        std::ofstream out(path, std::ios::binary);
        if (out.is_open()) {
            out.write(data + i * SECTOR_SIZE, SECTOR_SIZE);
            out.close();
        }
    }
}

void BufferManager::printBufferTable() {
    std::cout << std::left;
    std::cout << "┌─────────────────────────────────────────────────────────────────────────────────────────┐\n";
    std::cout << "│ ID Frame                   │ ID Página              │ PinCount │ Dirty │ LRU Pos │ Mode │\n";
    std::cout << "├─────────────────────────────────────────────────────────────────────────────────────────┤\n";

    for (const auto& par : pageTable) {
        const IDPagina& id = par.first;
        const BufferFrame* frame = par.second;

        int lruPosition = -1;
        auto it = lruMap.find(id);
        if (it != lruMap.end()) {
            lruPosition = std::distance(lruList.begin(), it->second);
        }

        std::ostringstream framePtr;
        framePtr << std::hex << std::showbase << (uintptr_t)frame;

        std::ostringstream idStream;
        idStream << id.plato << "," << id.cara << "," << id.pista << "," << id.sector;

        std::cout << "│ " << std::setw(26) << framePtr.str()
                  << "│ " << std::setw(24) << idStream.str()
                  << "│ " << std::setw(8)  << frame->pinCount
                  << " │ " << std::setw(5) << (frame->dirty ? "Yes" : "No")
                  << " │ " << std::setw(7) << lruPosition
                  << " │ " << std::setw(4) << (frame->isWrite ? "W" : "L")
                  << " │\n";
    }

    std::cout << "└────────────────────────────────────────────────────────────────────────────────────────────┘\n";
}


