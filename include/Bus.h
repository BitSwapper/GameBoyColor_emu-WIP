#ifndef BUS_H
#define BUS_H

#include <cstdint>
#include <array>   
#include <memory>  

class Cartridge;

class Bus {
public:
    Bus();

    void connectCartridge(const std::shared_ptr<Cartridge>& cartridge);
    uint8_t read(uint16_t address);
    void write(uint16_t address, uint8_t value);
    void reset();

private:
    std::shared_ptr<Cartridge> cartridge_;
    std::array<uint8_t, 8 * 1024> wram_;
    std::array<uint8_t, 127> hram_;
    uint8_t interrupt_enable_register_;

};

#endif 