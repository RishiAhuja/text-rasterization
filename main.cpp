#include "ttf_reader.h"

int main() {
    TTFReader reader;
    
    if (!reader.openFont("JetBrainsMono-Bold.ttf")) {
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
            auto glyfData = reader.readBytes(200);  // Read first 200 bytes
            reader.printHexDump(glyfData, glyfEntry.offset);
        }
    } else {
        std::cout << "'glyf' table not found!" << std::endl;
    }
    return 0;
}