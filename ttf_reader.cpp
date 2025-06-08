#include "ttf_reader.h"
#include <iomanip>

TTFReader::TTFReader() {
    littleEndian = isLittleEndian();
}

TTFReader::~TTFReader() {
    close();
}

bool TTFReader::isLittleEndian() {
    uint16_t test = 1;
    return reinterpret_cast<uint8_t*>(&test)[0] == 1;
}

uint16_t TTFReader::swapUint16(uint16_t val) {
    return (val << 8) | (val >> 8);
}

uint32_t TTFReader::swapUint32(uint32_t val) {
    return (val << 24) | ((val & 0x0000FF00) << 8) | 
           ((val & 0x00FF0000) >> 8) | (val >> 24);
}

bool TTFReader::openFont(const std::string& filename) {
    file.open(filename, std::ios::binary);
    return file.is_open();
}

void TTFReader::close() {
    if (file.is_open()) {
        file.close();
    }
}

bool TTFReader::readHeader(TTFHeader& header) {
    if (!file.good()) return false;
    
    file.read(reinterpret_cast<char*>(&header.scalerType), 4);
    file.read(reinterpret_cast<char*>(&header.numTables), 2);
    file.read(reinterpret_cast<char*>(&header.searchRange), 2);
    file.read(reinterpret_cast<char*>(&header.entrySelector), 2);
    file.read(reinterpret_cast<char*>(&header.rangeShift), 2);
    
    if (littleEndian) {
        header.scalerType = swapUint32(header.scalerType);
        header.numTables = swapUint16(header.numTables);
        header.searchRange = swapUint16(header.searchRange);
        header.entrySelector = swapUint16(header.entrySelector);
        header.rangeShift = swapUint16(header.rangeShift);
    }
    
    return file.good();
}

bool TTFReader::readTableEntry(TableEntry& entry) {
    if (!file.good()) return false;
    
    file.read(entry.tag, 4);
    entry.tag[4] = '\0';  // Null terminate
    
    file.read(reinterpret_cast<char*>(&entry.checksum), 4);
    file.read(reinterpret_cast<char*>(&entry.offset), 4);
    file.read(reinterpret_cast<char*>(&entry.length), 4);
    
    if (littleEndian) {
        entry.checksum = swapUint32(entry.checksum);
        entry.offset = swapUint32(entry.offset);
        entry.length = swapUint32(entry.length);
    }
    
    return file.good();
}

void TTFReader::printHeader(const TTFHeader& header) {
    std::cout << "TTF Header:" << std::endl;
    std::cout << "  scalerType: 0x" << std::hex << header.scalerType << std::dec;
    if (header.scalerType == 0x00010000) {
        std::cout << " (TrueType)" << std::endl;
    } else {
        std::cout << " (Unknown)" << std::endl;
    }
    std::cout << "  numTables: " << header.numTables << std::endl;
}

void TTFReader::printTableEntry(const TableEntry& entry) {
    std::cout << "  Table: " << entry.tag 
              << ", Offset: " << entry.offset 
              << ", Length: " << entry.length << std::endl;
}


bool TTFReader::findTable(const std::string& tableName, TableEntry& entry) {
    // Reset to start of file and skip header
    file.seekg(12, std::ios::beg);
    
    TTFHeader header;
    file.seekg(0, std::ios::beg);
    if (!readHeader(header)) return false;
    
    // Search through table entries
    for (int i = 0; i < header.numTables; i++) {
        TableEntry tempEntry;
        if (readTableEntry(tempEntry)) {
            if (std::string(tempEntry.tag) == tableName) {
                entry = tempEntry;
                return true;
            }
        }
    }
    return false;
}

bool TTFReader::seekToTable(const std::string& tableName) {
    TableEntry entry;
    if (findTable(tableName, entry)) {
        file.seekg(entry.offset, std::ios::beg);
        return true;
    }
    return false;
}

std::vector<uint8_t> TTFReader::readBytes(size_t count) {
    std::vector<uint8_t> data(count);
    file.read(reinterpret_cast<char*>(data.data()), count);
    
    // Resize to actual bytes read
    data.resize(file.gcount());
    return data;
}

void TTFReader::printHexDump(const std::vector<uint8_t>& data, size_t offset) {
    std::cout << "Hex dump (" << data.size() << " bytes):" << std::endl;
    
    for (size_t i = 0; i < data.size(); i += 16) {
        // Print offset
        std::cout << std::setfill('0') << std::setw(8) << std::hex << (offset + i) << ": ";
        
        // Print hex bytes
        for (size_t j = 0; j < 16 && (i + j) < data.size(); j++) {
            std::cout << std::setfill('0') << std::setw(2) << std::hex 
                      << static_cast<int>(data[i + j]) << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::dec; // Reset to decimal
}