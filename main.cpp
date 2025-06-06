#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>

using namespace std;

bool is_little_endian() {
    uint16_t test = 1; // 0x0001
    return reinterpret_cast<uint8_t*>(&test)[0] == 1;
}

uint16_t swap_uint16(uint16_t val) {
    return (val << 8) | (val >> 8);
}

// Byte swap for 32-bit unsigned integer
uint32_t swap_uint32(uint32_t val) {
    return (val << 24) | ((val & 0x0000FF00) << 8) | ((val & 0x00FF0000) >> 8) | (val >> 24);
}

int main() {
    const char* filename = "JetBrainsMono-Bold.ttf";

    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "Failed to open font file: " << filename << std::endl;
        return 1;
    }

    // std::cout << "Reading first 100 bytes of '" << filename << "':" << std::endl;
    // for (int i = 0; i < 100 && file.good(); ++i) {
    //     uint8_t byte;
    //     file.read(reinterpret_cast<char*>(&byte), sizeof(byte));

    //     std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";

    //     if ((i + 1) % 16 == 0) {
    //         std::cout << std::endl;
    //     }
    // }
    // std::cout << std::dec << std::endl;

    uint32_t scalerType_raw;
    uint32_t scalerType = is_little_endian() ? swap_uint32(scalerType_raw) : scalerType_raw;
    uint16_t numTables_raw;
    uint16_t numTables = is_little_endian() ? swap_uint16(numTables_raw) : numTables_raw;

    file.read(reinterpret_cast<char*>(&scalerType_raw), sizeof(scalerType_raw));
    scalerType = is_little_endian() ? swap_uint32(scalerType_raw) : scalerType_raw;
    
    std::cout << "From file, scalerType (raw hex): 0x" << std::hex << std::setw(8) << std::setfill('0') << scalerType_raw << std::endl;
    std::cout << "From file, interpreted scalerType: 0x" << std::hex << std::setw(8) << std::setfill('0') << scalerType << std::dec << std::endl;

    // Read numTables (uint16)
    file.read(reinterpret_cast<char*>(&numTables_raw), sizeof(numTables_raw));
    numTables = is_little_endian() ? swap_uint16(numTables_raw) : numTables_raw;

    std::cout << "From file, numTables (raw hex): 0x" << std::hex << std::setw(4) << std::setfill('0') << numTables_raw << std::endl;
    std::cout << "From file, interpreted numTables: " << std::dec << numTables << std::endl;

    file.close();

    std::cout << "Finished reading." << std::endl;

    std::cout << is_little_endian() << "\n";

}