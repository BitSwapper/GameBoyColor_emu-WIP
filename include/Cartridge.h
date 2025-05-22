#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <string>
#include <vector>
#include <cstdint>

class Cartridge {
public:
    Cartridge();
    bool loadRom(const std::string& rom_path);
    bool loadTestData(const std::vector<uint8_t>& data);
    
    uint8_t read(uint16_t address) const;
    
    const std::vector<uint8_t>& getRomData() const;

private:
    std::vector<uint8_t> rom_data_;
};

#endif 