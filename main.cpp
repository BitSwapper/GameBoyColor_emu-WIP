#include "Config.h"
#include "Emulator.h"
#include <iostream>
#include <vector>
#include <string>

int main() {
    bool load_real_rom_on_startup = false;

    Emulator gbc_emulator;

    if (load_real_rom_on_startup) {
        std::string rom_path_str = Config::DEFAULT_ROM_PATH;
        if (rom_path_str.empty()) {
            std::cerr << "Error: DEFAULT_ROM_PATH in Config.h is not set." << std::endl;
            return 1;
        }
        std::cout << "GBC Emulator Starting with ROM: " << rom_path_str << std::endl;
        if (!gbc_emulator.initialize(rom_path_str)) {
            std::cerr << "Emulator initialization failed for ROM mode." << std::endl;
            return 1;
        }
    }
    else {
        std::cout << "GBC Emulator Starting. Load a test ROM via the ImGui Debug Controls." << std::endl;
        std::vector<uint8_t> initial_dummy_data = { 0x76 }; 
        if (!gbc_emulator.initialize(initial_dummy_data, 0x0000)) {
            std::cerr << "Initial (dummy) test load failed during startup." << std::endl;
            return 1;
        }
    }

    gbc_emulator.run();

    std::cout << "All operations finished." << std::endl;
    return 0;
}