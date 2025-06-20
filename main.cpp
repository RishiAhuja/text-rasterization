#include "ttf_reader.h"

int main() {
    TTFReader reader;
    
    if (!reader.openFont("RobotoMono-VariableFont_wght.ttf")) {
        std::cout << "Failed to open font file" << std::endl;
        return 1;
    }
    
    TTFHeader header;
    if (reader.readHeader(header)) {
        reader.printHeader(header);
        
        std::cout << "\nTables:" << std::endl;
        for (int i = 0; i < header.numTables; i++) {
            TableEntry entry;
            if (reader.readTableEntry(entry)) {
                reader.printTableEntry(entry);
            }
        }
    }
    std::cout << "\n--- Reading 'glyf' table ---" << std::endl;
    TableEntry glyfEntry;
    if (reader.findTable("glyf", glyfEntry)) {
        std::cout << "Found 'glyf' table at offset: " << glyfEntry.offset 
                  << ", length: " << glyfEntry.length << std::endl;
        
        if (reader.seekToTable("glyf")) {
            // Instead of hex dump, let's parse the first glyph
            std::cout << "\n--- Parsing first glyph ---" << std::endl;
            SimpleGlyph glyph;
            int glyphIndex = 3;
            // SimpleGlyph glyph2;
            // SimpleGlyph glyph3;
            // if (reader.readSimpleGlyph(glyph)) {
            //     reader.printGlyph(glyph);
            //     reader.plotGlyph(glyph);
            //     reader.exportGlyphSVG(glyph, "glyph.svg");
            // } else {
            //     std::cout << "Failed to parse glyph" << std::endl;
            // }
            // if (reader.readSimpleGlyph(glyph2)) {
            //     reader.printGlyph(glyph2);
            //     reader.plotGlyph(glyph2);
            //     reader.exportGlyphSVG(glyph2, "glyph2.svg");
            // } else {
            //     std::cout << "Failed to parse glyph" << std::endl;
            // }
            // if (reader.readSimpleGlyph(glyph3)) {
            //     reader.printGlyph(glyph3);
            //     reader.plotGlyph(glyph3);
            //     reader.exportGlyphSVG(glyph3, "glyph3.svg");
            // } else {
            //     std::cout << "Failed to parse glyph" << std::endl;
            // }
            //  reader.readMultipleGlyphs(5);
            std::cout << "\n=== Reading Glyphs by Index ===" << std::endl;
            // reader.readMultipleGlyphsByIndex(1, 26);  // Read glyphs 1-10
            if (reader.readGlyphByIndex(glyphIndex, glyph)) {
                reader.printGlyph(glyph);
                reader.plotGlyph(glyph);

                // Export both versions
                std::string basicFilename = "glyph_" + std::to_string(glyphIndex) + "_basic.svg";
                std::string curveFilename = "glyph_" + std::to_string(glyphIndex) + "_curves.svg";

                reader.exportGlyphSVG(glyph, basicFilename);           // Original (straight lines)
                reader.exportGlyphWithCurves(glyph, curveFilename);    // With Bézier curves
            }
        }
    } else {
        std::cout << "'glyf' table not found!" << std::endl;
    }
    
    return 0;
}