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

void TTFReader::plotGlyph(const SimpleGlyph& glyph, int width, int height) {
    if (glyph.points.empty()) return;
    
    std::vector<std::vector<char>> grid(height, std::vector<char>(width, ' '));
    
    int16_t xRange = glyph.header.xMax - glyph.header.xMin;
    int16_t yRange = glyph.header.yMax - glyph.header.yMin;
    
    if (xRange == 0 || yRange == 0) return;
    
    for (const auto& point : glyph.points) {
        int x = ((point.x - glyph.header.xMin) * (width - 1)) / xRange;
        int y = height - 1 - ((point.y - glyph.header.yMin) * (height - 1)) / yRange;
        
        // Bounds check
        if (x >= 0 && x < width && y >= 0 && y < height) {
            grid[y][x] = point.onCurve ? '*' : 'o';
        }
    }
    
    // Print grid
    std::cout << "\nGlyph Plot (" << width << "x" << height << "):" << std::endl;
    std::cout << "* = on-curve points, o = off-curve (control) points" << std::endl;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            std::cout << grid[y][x];
        }
        std::cout << std::endl;
    }
}

void TTFReader::exportGlyphSVG(const SimpleGlyph& glyph, const std::string& filename) {
    std::ofstream svg(filename);
    
    int width = glyph.header.xMax - glyph.header.xMin + 100;
    int height = glyph.header.yMax - glyph.header.yMin + 100;
    
    svg << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    svg << "<svg width=\"" << width << "\" height=\"" << height << "\" xmlns=\"http://www.w3.org/2000/svg\">\n";
    svg << "<g transform=\"translate(50," << (height - 50) << ") scale(1,-1)\">\n";
    
    // Draw points
    for (size_t i = 0; i < glyph.points.size(); i++) {
        const auto& point = glyph.points[i];
        int x = point.x - glyph.header.xMin;
        int y = point.y - glyph.header.yMin;
        
        if (point.onCurve) {
            svg << "<circle cx=\"" << x << "\" cy=\"" << y << "\" r=\"3\" fill=\"red\"/>\n";
        } else {
            svg << "<circle cx=\"" << x << "\" cy=\"" << y << "\" r=\"2\" fill=\"blue\"/>\n";
        }
        
        svg << "<text x=\"" << (x + 5) << "\" y=\"" << (y + 5) << "\" font-size=\"8\" fill=\"white\" transform=\"scale(1,-1)\">" << i << "</text>\n";
    }
    
    size_t pointIndex = 0;
    for (size_t contour = 0; contour < glyph.endPtsOfContours.size(); contour++) {
        size_t startPt = pointIndex;
        size_t endPt = glyph.endPtsOfContours[contour];
        
        svg << "<polygon points=\"";
        while (pointIndex <= endPt) {
            const auto& point = glyph.points[pointIndex];
            int x = point.x - glyph.header.xMin;
            int y = point.y - glyph.header.yMin;
            svg << x << "," << y << " ";
            pointIndex++;
        }
        svg << "\" fill=\"none\" stroke=\"green\" stroke-width=\"1\"/>\n";
    }
    
    svg << "</g>\n</svg>\n";
    svg.close();
    
    std::cout << "SVG exported to: " << filename << std::endl;
}



bool TTFReader::readMultipleGlyphs(int count) {
    for (int i = 0; i < count; i++) {
        std::cout << "\n--- Parsing glyph " << i << " ---" << std::endl;
        
        // Save current position
        std::streampos currentPos = file.tellg();
        
        SimpleGlyph glyph;
        if (readSimpleGlyph(glyph)) {
            std::cout << "Glyph " << i << ":" << std::endl;
            printGlyph(glyph);
            plotGlyph(glyph);
            
            std::string filename = "glyph_" + std::to_string(i) + ".svg";
            exportGlyphSVG(glyph, filename);
        } else {
            std::cout << "Failed to parse glyph " << i << " - trying to skip ahead" << std::endl;
            
            // Go back to where we started and skip a reasonable amount
            file.clear(); // Clear any error flags
            file.seekg(currentPos + std::streamoff(100), std::ios::beg); // Skip 100 bytes
            
            if (!file.good()) {
                std::cout << "Reached end of glyph data" << std::endl;
                break;
            }
        }
    }
    return true;
}

void TTFReader::explainLocaTable() {
    std::cout << "\n=== Understanding 'loca' table ===" << std::endl;
    
    // Step 1: Check format in head table
    TableEntry headEntry;
    if (!findTable("head", headEntry)) {
        std::cout << "No 'head' table found" << std::endl;
        return;
    }
    
    std::cout << "Found 'head' table at offset: " << headEntry.offset << std::endl;
    
    // Read indexToLocFormat (at offset 50 in head table)
    file.seekg(headEntry.offset + 50, std::ios::beg);
    int16_t indexToLocFormat;
    file.read(reinterpret_cast<char*>(&indexToLocFormat), 2);
    if (littleEndian) indexToLocFormat = static_cast<int16_t>(swapUint16(indexToLocFormat));
    
    bool isLongFormat = (indexToLocFormat == 1);
    std::cout << "Format: " << (isLongFormat ? "Long (4 bytes)" : "Short (2 bytes)") << std::endl;
    
    // Step 2: Read loca table
    TableEntry locaEntry;
    if (!findTable("loca", locaEntry)) {
        std::cout << "No 'loca' table found" << std::endl;
        return;
    }
    
    std::cout << "Found 'loca' table at offset: " << locaEntry.offset 
              << ", length: " << locaEntry.length << std::endl;
    
    // Calculate number of glyphs
    size_t entrySize = isLongFormat ? 4 : 2;
    size_t numEntries = locaEntry.length / entrySize;
    size_t numGlyphs = numEntries - 1; // Last entry is end marker
    
    std::cout << "Number of glyphs: " << numGlyphs << std::endl;
    
    // Read first 10 entries as example
    file.seekg(locaEntry.offset, std::ios::beg);
    std::cout << "\nFirst 10 glyph locations:" << std::endl;
    
    for (int i = 0; i < 10 && i < static_cast<int>(numEntries); i++) {
        uint32_t offset;
        
        if (isLongFormat) {
            file.read(reinterpret_cast<char*>(&offset), 4);
            if (littleEndian) offset = swapUint32(offset);
        } else {
            uint16_t shortOffset;
            file.read(reinterpret_cast<char*>(&shortOffset), 2);
            if (littleEndian) shortOffset = swapUint16(shortOffset);
            offset = shortOffset * 2; // Convert to actual offset
        }
        
        std::cout << "  Glyph " << i << ": starts at offset " << offset << std::endl;
    }
}



bool TTFReader::loadLocaTable() {
    // Step 1: Get format from head table
    TableEntry headEntry;
    if (!findTable("head", headEntry)) return false;
    
    file.seekg(headEntry.offset + 50, std::ios::beg);
    int16_t indexToLocFormat;
    file.read(reinterpret_cast<char*>(&indexToLocFormat), 2);
    if (littleEndian) indexToLocFormat = static_cast<int16_t>(swapUint16(indexToLocFormat));
    
    isLongFormat = (indexToLocFormat == 1);
    
    // Step 2: Load all glyph offsets
    TableEntry locaEntry;
    if (!findTable("loca", locaEntry)) return false;
    
    size_t entrySize = isLongFormat ? 4 : 2;
    size_t numEntries = locaEntry.length / entrySize;
    
    glyphOffsets.clear();
    glyphOffsets.reserve(numEntries);
    
    file.seekg(locaEntry.offset, std::ios::beg);
    
    for (size_t i = 0; i < numEntries; i++) {
        uint32_t offset;
        
        if (isLongFormat) {
            file.read(reinterpret_cast<char*>(&offset), 4);
            if (littleEndian) offset = swapUint32(offset);
        } else {
            uint16_t shortOffset;
            file.read(reinterpret_cast<char*>(&shortOffset), 2);
            if (littleEndian) shortOffset = swapUint16(shortOffset);
            offset = shortOffset * 2;
        }
        
        glyphOffsets.push_back(offset);
    }
    
    std::cout << "Loaded " << (numEntries - 1) << " glyph locations" << std::endl;
    return true;
}


bool TTFReader::readGlyphByIndex(int glyphIndex, SimpleGlyph& glyph) {
    if (glyphOffsets.empty()) {
        if (!loadLocaTable()) return false;
    }

    if (glyphIndex < 0 || glyphIndex >= static_cast<int>(glyphOffsets.size() - 1)) {
        std::cout << "Glyph index " << glyphIndex << " out of range" << std::endl;
        return false;
    }
    
    uint32_t glyphOffset = glyphOffsets[glyphIndex];
    uint32_t nextGlyphOffset = glyphOffsets[glyphIndex + 1];
    
    if (glyphOffset == nextGlyphOffset) {
        std::cout << "Glyph " << glyphIndex << " is empty (no outline data)" << std::endl;
        return false;
    }
    
    TableEntry glyfEntry;
    if (!findTable("glyf", glyfEntry)) return false;
    
    file.seekg(glyfEntry.offset + glyphOffset, std::ios::beg);
    
    std::cout << "Reading glyph " << glyphIndex << " at offset " << (glyfEntry.offset + glyphOffset) 
              << " (size: " << (nextGlyphOffset - glyphOffset) << " bytes)" << std::endl;
    
    return readSimpleGlyph(glyph);
}


void TTFReader::readMultipleGlyphsByIndex(int startIndex, int count) {
    for (int i = 0; i < count; i++) {
        int glyphIndex = startIndex + i;
        std::cout << "\n=== Reading Glyph " << glyphIndex << " ===" << std::endl;
        
        SimpleGlyph glyph;
        if (readGlyphByIndex(glyphIndex, glyph)) {
            printGlyph(glyph);
            plotGlyph(glyph);
            
            std::string filename = "glyph_" + std::to_string(glyphIndex) + ".svg";
            exportGlyphSVG(glyph, filename);
        } else {
            std::cout << "Failed to read glyph " << glyphIndex << std::endl;
        }
    }
}


BezierPoint TTFReader::lerp(const BezierPoint& p1, const BezierPoint& p2, float t) {
    return BezierPoint(
        p1.x + t * (p2.x - p1.x),
        p1.y + t * (p2.y - p1.y)
    );
}

BezierPoint TTFReader::quadraticBezier(const BezierPoint& start, const BezierPoint& control, const BezierPoint& end, float t) {
    BezierPoint p01 = lerp(start, control, t);    // Lerp between start and control
    BezierPoint p12 = lerp(control, end, t);      // Lerp between control and end
    return lerp(p01, p12, t);                     // Lerp between the two results
}


std::vector<BezierPoint> TTFReader::generateBezierCurve(const BezierPoint& start, const BezierPoint& control, const BezierPoint& end, int resolution) {
    std::vector<BezierPoint> points;
    points.reserve(resolution + 1);
    
    for (int i = 0; i <= resolution; i++) {
        float t = static_cast<float>(i) / resolution;
        points.push_back(quadraticBezier(start, control, end, t));
    }
    
    return points;
}


std::vector<BezierPoint> TTFReader::generateGlyphOutline(const SimpleGlyph& glyph, int resolution) {
    std::vector<BezierPoint> outline;
    
    size_t pointIndex = 0;
    
    // Process each contour
    for (size_t contour = 0; contour < glyph.endPtsOfContours.size(); contour++) {
        size_t startPt = pointIndex;
        size_t endPt = glyph.endPtsOfContours[contour];
        
        std::cout << "Processing contour " << contour << " (points " << startPt << " to " << endPt << ")" << std::endl;
        
        // Process points in this contour
        while (pointIndex <= endPt) {
            const Point& currentPt = glyph.points[pointIndex];
            
            if (currentPt.onCurve) {
                // On-curve point - add directly to outline
                outline.push_back(BezierPoint(currentPt.x, currentPt.y));
                pointIndex++;
            } else {
                // Off-curve point - this is a control point for a Bézier curve
                if (pointIndex == 0 || pointIndex >= glyph.points.size() - 1) {
                    // Edge case: skip if we can't form a proper curve
                    pointIndex++;
                    continue;
                }
                
                // Find the previous on-curve point
                BezierPoint startPoint;
                if (outline.empty()) {
                    // Use the last point of this contour as start
                    const Point& lastPt = glyph.points[endPt];
                    startPoint = BezierPoint(lastPt.x, lastPt.y);
                } else {
                    startPoint = outline.back();
                }
                
                // Current point is the control point
                BezierPoint controlPoint(currentPt.x, currentPt.y);
                
                // Find the next on-curve point (or implied point)
                BezierPoint endPoint;
                if (pointIndex + 1 <= endPt && glyph.points[pointIndex + 1].onCurve) {
                    // Next point is on-curve
                    const Point& nextPt = glyph.points[pointIndex + 1];
                    endPoint = BezierPoint(nextPt.x, nextPt.y);
                } else if (pointIndex + 1 <= endPt) {
                    // Next point is also off-curve, so we have an implied on-curve point
                    const Point& nextPt = glyph.points[pointIndex + 1];
                    endPoint = BezierPoint(
                        (currentPt.x + nextPt.x) / 2.0f,
                        (currentPt.y + nextPt.y) / 2.0f
                    );
                } else {
                    // Wrap to start of contour
                    const Point& firstPt = glyph.points[startPt];
                    endPoint = BezierPoint(firstPt.x, firstPt.y);
                }
                
                // Generate the Bézier curve
                std::vector<BezierPoint> curvePoints = generateBezierCurve(startPoint, controlPoint, endPoint, resolution);
                
                // Add curve points to outline (skip first point to avoid duplication)
                for (size_t i = 1; i < curvePoints.size(); i++) {
                    outline.push_back(curvePoints[i]);
                }
                
                pointIndex++;
            }
        }
    }
    
    return outline;

}



// void TTFReader::exportGlyphWithCurves(const SimpleGlyph& glyph, const std::string& filename) {
//     std::ofstream svg(filename);
    
//     int width = glyph.header.xMax - glyph.header.xMin + 100;
//     int height = glyph.header.yMax - glyph.header.yMin + 100;
    
//     svg << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
//     svg << "<svg width=\"" << width << "\" height=\"" << height << "\" xmlns=\"http://www.w3.org/2000/svg\">\n";
//     svg << "<g transform=\"translate(50," << (height - 50) << ") scale(1,-1)\">\n";
    
//     // Draw original points (for reference)
//     for (size_t i = 0; i < glyph.points.size(); i++) {
//         const auto& point = glyph.points[i];
//         int x = point.x - glyph.header.xMin;
//         int y = point.y - glyph.header.yMin;
        
//         if (point.onCurve) {
//             svg << "<circle cx=\"" << x << "\" cy=\"" << y << "\" r=\"2\" fill=\"red\" opacity=\"0.7\"/>\n";
//         } else {
//             svg << "<circle cx=\"" << x << "\" cy=\"" << y << "\" r=\"1\" fill=\"blue\" opacity=\"0.7\"/>\n";
//         }
//     }
    
//     // Generate and draw smooth outline
//     std::vector<BezierPoint> outline = generateGlyphOutline(glyph, 10);
    
//     if (!outline.empty()) {
//         svg << "<polyline points=\"";
//         for (const auto& point : outline) {
//             int x = static_cast<int>(point.x - glyph.header.xMin);
//             int y = static_cast<int>(point.y - glyph.header.yMin);
//             svg << x << "," << y << " ";
//         }
//         svg << "\" fill=\"none\" stroke=\"green\" stroke-width=\"2\"/>\n";
//     }
    
//     svg << "</g>\n</svg>\n";
//     svg.close();
    
//     std::cout << "Smooth curve SVG exported to: " << filename << std::endl;
// }



// Keep the existing working exportGlyphSVG function as is, and add this new function:

void TTFReader::exportGlyphWithCurves(const SimpleGlyph& glyph, const std::string& filename) {
    std::ofstream svg(filename);
    
    int width = glyph.header.xMax - glyph.header.xMin + 100;
    int height = glyph.header.yMax - glyph.header.yMin + 100;
    
    svg << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    svg << "<svg width=\"" << width << "\" height=\"" << height << "\" xmlns=\"http://www.w3.org/2000/svg\">\n";
    svg << "<g transform=\"translate(50," << (height - 50) << ") scale(1,-1)\">\n";
    
    // Draw original points for reference
    for (size_t i = 0; i < glyph.points.size(); i++) {
        const auto& point = glyph.points[i];
        int x = point.x - glyph.header.xMin;
        int y = point.y - glyph.header.yMin;
        
        if (point.onCurve) {
            svg << "<circle cx=\"" << x << "\" cy=\"" << y << "\" r=\"2\" fill=\"red\" opacity=\"0.5\"/>\n";
        } else {
            svg << "<circle cx=\"" << x << "\" cy=\"" << y << "\" r=\"1\" fill=\"blue\" opacity=\"0.5\"/>\n";
        }
        svg << "<text x=\"" << (x + 3) << "\" y=\"" << (y + 3) << "\" font-size=\"6\" fill=\"black\" transform=\"scale(1,-1)\">" << i << "</text>\n";
    }
    
    // Process each contour separately
    size_t pointIndex = 0;
    for (size_t contour = 0; contour < glyph.endPtsOfContours.size(); contour++) {
        size_t startPt = pointIndex;
        size_t endPt = glyph.endPtsOfContours[contour];
        
        svg << "<path d=\"";
        
        const auto& firstPoint = glyph.points[startPt];
        int startX = firstPoint.x - glyph.header.xMin;
        int startY = firstPoint.y - glyph.header.yMin;
        svg << "M " << startX << " " << startY;
        
        // Process each point in the contour
        for (size_t i = startPt; i <= endPt; i++) {
            size_t nextIndex = (i == endPt) ? startPt : i + 1;  // Wrap around at end
            
            const auto& currentPt = glyph.points[i];
            const auto& nextPt = glyph.points[nextIndex];
            
            int nextX = nextPt.x - glyph.header.xMin;
            int nextY = nextPt.y - glyph.header.yMin;
            
            if (nextPt.onCurve) {
                // Straight line to next on-curve point
                svg << " L " << nextX << " " << nextY;
            } else {
                // Quadratic curve - find the end point
                size_t endIndex = (nextIndex == endPt) ? startPt : nextIndex + 1;
                if (endIndex > endPt) endIndex = startPt;
                
                const auto& endPoint = glyph.points[endIndex];
                int endX, endY;
                
                if (endPoint.onCurve) {
                    // Next point after control is on-curve
                    endX = endPoint.x - glyph.header.xMin;
                    endY = endPoint.y - glyph.header.yMin;
                    i++; // Skip the control point in next iteration
                } else {
                    // Two consecutive off-curve points = implied on-curve point
                    endX = (nextPt.x + endPoint.x) / 2 - glyph.header.xMin;
                    endY = (nextPt.y + endPoint.y) / 2 - glyph.header.yMin;
                    // Don't skip - the next off-curve point will be processed next
                }
                
                // Control point
                int ctrlX = nextPt.x - glyph.header.xMin;
                int ctrlY = nextPt.y - glyph.header.yMin;
                
                svg << " Q " << ctrlX << " " << ctrlY << " " << endX << " " << endY;
            }
        }
        
        svg << " Z\" fill=\"none\" stroke=\"green\" stroke-width=\"2\"/>\n";  // Z closes the path
        
        pointIndex = endPt + 1;
    }
    
    svg << "</g>\n</svg>\n";
    svg.close();
    
    std::cout << "Curve SVG exported to: " << filename << std::endl;
}