#ifndef EMULATOR_UI_H
#define EMULATOR_UI_H

#include <string>
#include <vector>
#include <memory>
#include <functional> 


class Cpu;
class Bus;
class TestSuite;
struct TestRom;


struct SDL_Window;
typedef void* SDL_GLContext;


struct CpuDebugState {
    uint16_t af = 0, bc = 0, de = 0, hl = 0;
    uint16_t sp = 0, pc = 0;

    uint16_t last_instr_pc = 0;
    uint8_t  last_opcode = 0;
    uint16_t last_operand = 0; 
    uint8_t  last_instr_length = 0;
    std::string last_disassembled_str;
    std::vector<uint8_t> last_instr_bytes_vec;

    void capture(const Cpu& cpu_obj); 
};


class EmulatorUI {
public:
    EmulatorUI(
        Cpu& cpu_ref,
        Bus& bus_ref,
        TestSuite& ts_ref,
        std::string& rom_info_ref,
        bool& paused_ref,
        bool& step_req_ref,
        bool& emu_is_running_ref, 
        std::function<void(const TestRom&)> load_test_rom_fn,
        std::function<void()> reset_cpu_fn
    );
    ~EmulatorUI();

    bool initialize(); 
    void shutdown();   

    void processInput(); 
    void render();       

    void captureCpuStateForDiff();   
    void resetDisassemblyViewToPc(); 

private:
    
    SDL_Window* window_;
    SDL_GLContext gl_context_;

    uint16_t memory_editor_address_ = 0x0000; 
    char memory_editor_addr_input_buf_[5] = "0000"; 
    int memory_editor_bytes_per_row_ = 16;
    int memory_editor_num_rows_ = 16; 


    
    bool initSdlAndOpenGL();
    void initImGui();
    void cleanupSdl();
    void cleanupImGui();

    
    void drawCpuRegistersAndStateWindow();
    void drawDebugControlsWindow();
    void drawDisassemblyContextWindow();
    void drawStackViewWindow();
    void drawMemoryViewerWindow(); 
    void renderGBCFrame(); 

    
    Cpu& cpu_;
    Bus& bus_;
    TestSuite& test_suite_;
    std::string& current_rom_info_;
    bool& is_paused_for_step_;
    bool& step_requested_;
    bool& emulator_is_running_; 

    
    std::function<void(const TestRom&)> load_test_rom_callback_;
    std::function<void()> reset_cpu_callback_;

    
    CpuDebugState cpu_state_prev_frame_;

    
    uint16_t disassembly_view_center_address_ = 0x0100;
    bool follow_pc_in_disassembly_ = true;

    
    static const int INITIAL_WINDOW_WIDTH = 1280;
    static const int INITIAL_WINDOW_HEIGHT = 720;
};

#endif 