#ifndef EMULATOR_H
#define EMULATOR_H

#include <string>
#include <memory>
#include <vector>
#include "TestSuite.h" 
#include "Cpu.h"       


class Bus;
class Cartridge;
class EmulatorUI; 

class Emulator {
public:
    Emulator();
    ~Emulator();

    bool initialize(const std::string& rom_path);
    bool initialize(const std::vector<uint8_t>& test_data, uint16_t initial_pc = 0x0000);

    void run();

    void printCpuStateForDebug() const;

private:
    void step(); 

    
    std::shared_ptr<Cartridge> cartridge_;
    std::shared_ptr<Bus> bus_;
    std::unique_ptr<Cpu> cpu_;

    TestSuite test_suite_; 
    std::string current_rom_info_; 
    
    std::unique_ptr<EmulatorUI> ui_;
    
    bool is_initialized_ = false;
    bool is_running_ = false;
    bool is_paused_for_step_ = true; 
    bool step_requested_ = false;   

    bool coreInitialize(const std::shared_ptr<Cartridge>& cart, uint16_t initial_pc, const std::string& rom_info);
    
    void uiLoadTestRom(const TestRom& test_rom_struct);
    void uiResetCpu();
};

#endif 