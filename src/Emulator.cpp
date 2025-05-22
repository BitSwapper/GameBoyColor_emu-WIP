#include "Emulator.h"
#include "EmulatorUI.h" 
#include "Cpu.h"
#include "Bus.h"
#include "Cartridge.h"
#include "Utils.h"     
#include "TestSuite.h" 

#include <SDL_timer.h> 

#include <iostream>
#include <iomanip> 

Emulator::Emulator()
    : cartridge_(nullptr), bus_(nullptr), cpu_(nullptr), ui_(nullptr),
    is_initialized_(false), is_running_(false),
    is_paused_for_step_(true), step_requested_(false),
    current_rom_info_("No ROM Loaded") {
}

Emulator::~Emulator() {
    if (ui_) {
        ui_->shutdown(); 
    }
    std::cout << "Emulator components deallocated." << std::endl;
}

bool Emulator::coreInitialize(const std::shared_ptr<Cartridge>& cart, uint16_t initial_pc, const std::string& rom_info) {
    cartridge_ = cart;
    current_rom_info_ = rom_info;

    if (!bus_) bus_ = std::make_shared<Bus>();
    bus_->connectCartridge(cartridge_);

    if (!cpu_) cpu_ = std::make_unique<Cpu>();
    cpu_->connectBus(bus_);

    cpu_->reset();
    cpu_->pc = initial_pc;

    std::cout << "\nEmulator Core Initialized with: " << current_rom_info_ << std::endl;
    std::cout << "PC set to 0x" << std::hex << initial_pc << std::dec << std::endl;
    printCpuStateForDebug();

    is_initialized_ = true;
    is_paused_for_step_ = true;
    step_requested_ = false;
    return true;
}

bool Emulator::initialize(const std::string& rom_path) {
    auto new_cart = std::make_shared<Cartridge>();
    if (!new_cart->loadRom(rom_path)) {
        std::cerr << "Emulator Error: Failed to load ROM from path: " << rom_path << std::endl;
        return false;
    }
    size_t last_slash = rom_path.find_last_of("/\\");
    std::string display_name = (last_slash == std::string::npos) ? rom_path : rom_path.substr(last_slash + 1);

    if (!coreInitialize(new_cart, 0x0100, "ROM: " + display_name)) {
        return false;
    }

    if (!ui_) {
        ui_ = std::make_unique<EmulatorUI>(
            *cpu_, *bus_, test_suite_, current_rom_info_,
            is_paused_for_step_, step_requested_, is_running_,
            [this](const TestRom& tr) { this->uiLoadTestRom(tr); },
            [this]() { this->uiResetCpu(); }
        );
    }
    if (!ui_->initialize()) {
        std::cerr << "Emulator UI initialization failed." << std::endl;
        return false;
    }
    ui_->resetDisassemblyViewToPc();

    return true;
}

bool Emulator::initialize(const std::vector<uint8_t>& test_data, uint16_t initial_pc) {
    auto test_cart = std::make_shared<Cartridge>();
    if (!test_cart->loadTestData(test_data)) {
        std::cerr << "Emulator Error: Failed to load anonymous test data." << std::endl;
        return false;
    }

    if (!coreInitialize(test_cart, initial_pc, "Anonymous Test Data")) {
        return false;
    }

    if (!ui_) {
        ui_ = std::make_unique<EmulatorUI>(
            *cpu_, *bus_, test_suite_, current_rom_info_,
            is_paused_for_step_, step_requested_, is_running_,
            [this](const TestRom& tr) { this->uiLoadTestRom(tr); },
            [this]() { this->uiResetCpu(); }
        );
    }
    if (!ui_->initialize()) {
        std::cerr << "Emulator UI initialization failed for test data." << std::endl;
        return false;
    }
    ui_->resetDisassemblyViewToPc();

    return true;
}

void Emulator::uiLoadTestRom(const TestRom& test_rom_struct) {
    std::cout << "Loading Test ROM via UI: " << test_rom_struct.name << std::endl;
    bus_->reset();

    auto new_test_cart = std::make_shared<Cartridge>();
    if (!new_test_cart->loadTestData(test_rom_struct.data)) {
        std::cerr << "Emulator Error: Failed to load data for test: " << test_rom_struct.name << std::endl;
        return;
    }
    if (!coreInitialize(new_test_cart, test_rom_struct.initial_pc, "Test: " + test_rom_struct.name)) {
        std::cerr << "Core re-initialization failed for test ROM: " << test_rom_struct.name << std::endl;
        return;
    }

    if (ui_ && cpu_) {
        ui_->captureCpuStateForDiff();
        ui_->resetDisassemblyViewToPc();
    }
    is_paused_for_step_ = true;
    step_requested_ = false;
}

void Emulator::uiResetCpu() {
    if (cpu_ && bus_ && cartridge_) {
        cpu_->reset();
        bus_->reset();
        std::cout << "CPU Reset requested by UI." << std::endl;
        printCpuStateForDebug();

        if (ui_) {
            ui_->captureCpuStateForDiff();
            ui_->resetDisassemblyViewToPc();
        }
        is_paused_for_step_ = true;
        step_requested_ = false;
    }
}


void Emulator::printCpuStateForDebug() const {
    if (cpu_) {
        
        printf("PC: %s AF: %s(%s %s) BC: %s(%s %s) DE: %s(%s %s) HL: %s(%s %s) SP: %s\n",
            formatHex16(cpu_->pc).c_str(),
            formatHex16(cpu_->af).c_str(), formatHex8(cpu_->a()).c_str(), formatHex8(cpu_->f()).c_str(),
            formatHex16(cpu_->bc).c_str(), formatHex8(cpu_->b()).c_str(), formatHex8(cpu_->c()).c_str(),
            formatHex16(cpu_->de).c_str(), formatHex8(cpu_->d()).c_str(), formatHex8(cpu_->e()).c_str(),
            formatHex16(cpu_->hl).c_str(), formatHex8(cpu_->h()).c_str(), formatHex8(cpu_->l()).c_str(),
            formatHex16(cpu_->sp).c_str());
    }
}

void Emulator::step() {
    if (!is_initialized_ || !cpu_ || !bus_) return;
    cpu_->step();
}

void Emulator::run() {
    if (!is_initialized_ || !ui_) {
        std::cerr << "Emulator Error: Not fully initialized. Call initialize() first." << std::endl;
        return;
    }
    is_running_ = true;
    std::cout << "\n--- Starting Emulation Main Loop (" << current_rom_info_ << ") ---" << std::endl;

    while (is_running_) {
        ui_->processInput();
        if (!is_running_) break;

        bool should_execute_cpu_step = false;
        if (!is_paused_for_step_) {
            should_execute_cpu_step = true;
        }
        else if (step_requested_) {
            should_execute_cpu_step = true;
        }

        if (should_execute_cpu_step) {
            if (cpu_ && bus_) {
                ui_->captureCpuStateForDiff();

                uint16_t pc_before_step = cpu_->pc;
                uint8_t opcode_about_to_execute = bus_->read(pc_before_step);

                step();

                if (is_paused_for_step_ && step_requested_) {
                    printf("(Prev PC: %s Op: %s) -> New PC: %s, AF: %s (Cyc: %d)\n",
                        formatHex16(pc_before_step).c_str(),
                        formatHex8(opcode_about_to_execute).c_str(),
                        formatHex16(cpu_->pc).c_str(),
                        formatHex16(cpu_->af).c_str(),
                        cpu_->current_instruction_cycles_);
                }
                step_requested_ = false;

                if (opcode_about_to_execute == 0x76) { 
                    std::cout << "HALT instruction encountered @ " << formatHex16(pc_before_step) << ". Emulation paused." << std::endl;
                    is_paused_for_step_ = true;
                }
            }
        }

        ui_->render();

        if (is_paused_for_step_ && !step_requested_) {
            SDL_Delay(16);
        }
        else if (!is_paused_for_step_) {
            
        }
    }

    std::cout << "\n--- Emulation Loop Finished ---" << std::endl;
    if (cpu_) {
        std::cout << "Total CPU cycles elapsed: " << cpu_->cycles_elapsed_total_ << std::endl;
    }
}