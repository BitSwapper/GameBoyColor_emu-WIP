#include "Bus.h"
#include "Cartridge.h" 
#include <iostream>    

Bus::Bus() : interrupt_enable_register_(0) {
    reset();
}

void Bus::connectCartridge(const std::shared_ptr<Cartridge>& cartridge) {
    cartridge_ = cartridge;
}

void Bus::reset()
{
    wram_.fill(0);
    hram_.fill(0);
}

uint8_t Bus::read(uint16_t address) {
    if (address >= 0x0000 && address <= 0x7FFF) {
        if (cartridge_) {
            return cartridge_->read(address);
        }
        return 0xFF;
    }
    else if (address >= 0x8000 && address <= 0x9FFF) {
        return 0xFF;
    }
    else if (address >= 0xA000 && address <= 0xBFFF) {
        if (cartridge_) {
        }
        return 0xFF;
    }
    else if (address >= 0xC000 && address <= 0xDFFF) {
        return wram_[address - 0xC000];
    }
    else if (address >= 0xE000 && address <= 0xFDFF) {
        return wram_[(address - 0xE000) % wram_.size()];
    }
    else if (address >= 0xFE00 && address <= 0xFE9F) {
        return 0xFF;
    }
    else if (address >= 0xFEA0 && address <= 0xFEFF) {
        return 0xFF;
    }
    else if (address >= 0xFF00 && address <= 0xFF7F) {
        return 0xFF;
    }
    else if (address >= 0xFF80 && address <= 0xFFFE) {
        return hram_[address - 0xFF80];
    }
    else if (address == 0xFFFF) {
        return interrupt_enable_register_;
    }

    std::cerr << "Warning: Unhandled bus read at address 0x" << std::hex << address << std::endl;
    return 0xFF;
}

void Bus::write(uint16_t address, uint8_t value) {
    if (address >= 0x0000 && address <= 0x7FFF) {
        if (cartridge_) {
        }
        return;
    }
    else if (address >= 0x8000 && address <= 0x9FFF) {
        return;
    }
    else if (address >= 0xA000 && address <= 0xBFFF) {
        if (cartridge_) {
        }
        return;
    }
    else if (address >= 0xC000 && address <= 0xDFFF) {
        wram_[address - 0xC000] = value;
        return;
    }
    else if (address >= 0xE000 && address <= 0xFDFF) {
        wram_[(address - 0xE000) % wram_.size()] = value;
        return;
    }
    else if (address >= 0xFE00 && address <= 0xFE9F) {
        return;
    }
    else if (address >= 0xFEA0 && address <= 0xFEFF) {
        return;
    }
    else if (address >= 0xFF00 && address <= 0xFF7F) {
        return;
    }
    else if (address >= 0xFF80 && address <= 0xFFFE) {
        hram_[address - 0xFF80] = value;
        return;
    }
    else if (address == 0xFFFF) {
        interrupt_enable_register_ = value;
        return;
    }

    std::cerr << "Warning: Unhandled bus write at address 0x" << std::hex << address
        << " value: 0x" << (int)value << std::endl;
}