#include "Cartridge.h"
#include <fstream>
#include <iostream>

Cartridge::Cartridge() {
}

bool Cartridge::loadRom(const std::string& rom_path) {
    std::ifstream rom_file(rom_path, std::ios::binary | std::ios::ate);

    if (!rom_file.is_open()) {
        std::cerr << "Error: Could not open ROM file: " << rom_path << std::endl;
        return false;
    }

    std::streamsize size = rom_file.tellg();
    rom_file.seekg(0, std::ios::beg);

    rom_data_.resize(static_cast<size_t>(size));
    if (size > 0) { 
        if (!rom_file.read(reinterpret_cast<char*>(rom_data_.data()), size)) {
            std::cerr << "Error: Could not read ROM file: " << rom_path << std::endl;
            rom_data_.clear();
            return false;
        }
    }


    rom_file.close();
    std::cout << "Successfully loaded ROM: " << rom_path << " (" << size << " bytes)" << std::endl;
    return true;
}

uint8_t Cartridge::read(uint16_t address) const {
    if (address < rom_data_.size()) {
        return rom_data_[address];
    }
    return 0xFF;
}

const std::vector<uint8_t>& Cartridge::getRomData() const {
    return rom_data_;
}

bool Cartridge::loadTestData(const std::vector<uint8_t>& data) {
    rom_data_ = data;
    if (rom_data_.empty()) {
        std::cout << "Warning: Loaded empty test data into cartridge." << std::endl;
    }
    else {
        std::cout << "Successfully loaded " << rom_data_.size() << " bytes of test data into cartridge." << std::endl;
    }
    return true;
}