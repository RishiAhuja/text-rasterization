#ifndef TTF_READER_H
#define TTF_READER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

// TTF Header (Offset Table)
struct TTFHeader {
    uint32_t scalerType;
    uint16_t numTables;
    uint16_t searchRange;
    uint16_t entrySelector;
    uint16_t rangeShift;
};

struct GlyphHeader {
    int16_t numberOfContours;
    int16_t xMin, yMin, xMax, yMax;
};

struct Point {
    int16_t x, y;
    bool onCurve;
};

struct SimpleGlyph {
    GlyphHeader header;
    std::vector<uint16_t> endPtsOfContours;
    std::vector<Point> points;
};

// Table Directory Entry
struct TableEntry {
    char tag[5];       
    uint32_t checksum;
    uint32_t offset;
    uint32_t length;
};

class TTFReader {
private:
    std::ifstream file;
    bool littleEndian;
    
    // Byte swapping utilities
    uint16_t swapUint16(uint16_t val);
    uint32_t swapUint32(uint32_t val);
    bool isLittleEndian();

public:
    TTFReader();
    ~TTFReader();
    
    bool openFont(const std::string& filename);
    void close();
    
    bool readHeader(TTFHeader& header);
    bool readTableEntry(TableEntry& entry);
    void printHeader(const TTFHeader& header);
    void printTableEntry(const TableEntry& entry);

    bool findTable(const std::string& tableName, TableEntry& entry);
    bool seekToTable(const std::string& tableName);
    std::vector<uint8_t> readBytes(size_t count);
    void printHexDump(const std::vector<uint8_t>& data, size_t offset = 0);

    // glphy reading functions
    bool readGlyphHeader(GlyphHeader& header);
    bool readSimpleGlyph(SimpleGlyph& glyph);
    void printGlyph(const SimpleGlyph& glyph);
    void plotGlyph(const SimpleGlyph& glyph, int width = 80, int height = 40);
    void exportGlyphSVG(const SimpleGlyph& glyph, const std::string& filename);
    bool readMultipleGlyphs(int count);
    void explainLocaTable();

};

#endif