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




bool TTFReader::readGlyphHeader(GlyphHeader& header) {
    if (!file.good()) return false;
    
    file.read(reinterpret_cast<char*>(&header.numberOfContours), 2);
    file.read(reinterpret_cast<char*>(&header.xMin), 2);
    file.read(reinterpret_cast<char*>(&header.yMin), 2);
    file.read(reinterpret_cast<char*>(&header.xMax), 2);
    file.read(reinterpret_cast<char*>(&header.yMax), 2);
    
    if (littleEndian) {
        header.numberOfContours = static_cast<int16_t>(swapUint16(header.numberOfContours));
        header.xMin = static_cast<int16_t>(swapUint16(header.xMin));
        header.yMin = static_cast<int16_t>(swapUint16(header.yMin));
        header.xMax = static_cast<int16_t>(swapUint16(header.xMax));
        header.yMax = static_cast<int16_t>(swapUint16(header.yMax));
    }
    return file.good();
}

bool TTFReader::readSimpleGlyph(SimpleGlyph& glyph) {
    if(!readGlyphHeader(glyph.header)) return false;

    if (glyph.header.numberOfContours < 0) {
        std::cout << "Composite glyph - not implemented yet" << std::endl;
        return false;
    }

    glyph.endPtsOfContours.resize(glyph.header.numberOfContours);
    for (int i = 0; i < glyph.header.numberOfContours; i++) {
        uint16_t endPt;
        file.read(reinterpret_cast<char*>(&endPt), 2);
        if (littleEndian) endPt = swapUint16(endPt);
        glyph.endPtsOfContours[i] = endPt;
    }

    uint16_t numPoints = glyph.endPtsOfContours.back() + 1;

    uint16_t instructionLength;
    file.read(reinterpret_cast<char*>(&instructionLength), 2);
    if (littleEndian) instructionLength = swapUint16(instructionLength);
    file.seekg(instructionLength, std::ios::cur); // Skip instructions
    
    std::vector<uint8_t> flags;
    flags.reserve(numPoints);

    for (uint16_t i = 0; i < numPoints; ) {
        uint8_t flag;
        file.read(reinterpret_cast<char*>(&flag), 1);
        flags.push_back(flag);
        i++;
        
        // Handle repeat flag
        if (flag & 0x08) { // REPEAT_FLAG
            uint8_t repeatCount;
            file.read(reinterpret_cast<char*>(&repeatCount), 1);
            for (int j = 0; j < repeatCount && i < numPoints; j++, i++) {
                flags.push_back(flag);
            }
        }
    }

    glyph.points.resize(numPoints);
    int16_t currentX = 0, currentY = 0;

    for (uint16_t i = 0; i < numPoints; i++) {
        uint8_t flag = flags[i];
        glyph.points[i].onCurve = (flag & 0x01) != 0;  

        if (flag & 0x02) { // X_SHORT_VECTOR
            uint8_t deltaX;
            file.read(reinterpret_cast<char*>(&deltaX), 1);
            currentX += (flag & 0x10) ? deltaX : -deltaX;
        } else if (!(flag & 0x10)) { // X coordinate changed
            int16_t deltaX;
            file.read(reinterpret_cast<char*>(&deltaX), 2);
            if (littleEndian) deltaX = static_cast<int16_t>(swapUint16(deltaX));
            currentX += deltaX;
        }

        glyph.points[i].x = currentX;
    }

     for (uint16_t i = 0; i < numPoints; i++) {
        uint8_t flag = flags[i];
        
        if (flag & 0x04) { // Y_SHORT_VECTOR
            uint8_t deltaY;
            file.read(reinterpret_cast<char*>(&deltaY), 1);
            currentY += (flag & 0x20) ? deltaY : -deltaY;
        } else if (!(flag & 0x20)) { // Y coordinate changed
            int16_t deltaY;
            file.read(reinterpret_cast<char*>(&deltaY), 2);
            if (littleEndian) deltaY = static_cast<int16_t>(swapUint16(deltaY));
            currentY += deltaY;
        }
        glyph.points[i].y = currentY;
    }
    return true;
}


void TTFReader::printGlyph(const SimpleGlyph& glyph) {
    std::cout << "Glyph Info:" << std::endl;
    std::cout << "  Contours: " << glyph.header.numberOfContours << std::endl;
    std::cout << "  Bounding box: (" << glyph.header.xMin << ", " << glyph.header.yMin 
              << ") to (" << glyph.header.xMax << ", " << glyph.header.yMax << ")" << std::endl;
    
    std::cout << "  End points: ";
    for (auto endPt : glyph.endPtsOfContours) {
        std::cout << endPt << " ";
    }
    std::cout << std::endl;
    
    std::cout << "  Points:" << std::endl;
    for (size_t i = 0; i < glyph.points.size(); i++) {
        std::cout << "    " << i << ": (" << glyph.points[i].x << ", " << glyph.points[i].y 
                  << ") " << (glyph.points[i].onCurve ? "ON" : "OFF") << std::endl;
    }
}