#include "EmulatorUI.h"
#include "Cpu.h"
#include "Bus.h"
#include "Utils.h"
#include "TestSuite.h"

#include <SDL.h>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif

#include <iostream>
#include <vector>
#include <cstdio>  

void CpuDebugState::capture(const Cpu& cpu_obj) {
    af = cpu_obj.af;
    bc = cpu_obj.bc;
    de = cpu_obj.de;
    hl = cpu_obj.hl;
    sp = cpu_obj.sp;
    pc = cpu_obj.pc;

    last_instr_pc = cpu_obj.debug_last_instr_pc_;
    last_opcode = cpu_obj.debug_last_opcode_;
    last_operand = cpu_obj.debug_last_operand_;
    last_instr_length = cpu_obj.debug_last_instr_length_;
    last_disassembled_str = cpu_obj.debug_last_disassembled_;
    last_instr_bytes_vec = cpu_obj.debug_last_instr_bytes_;
}



EmulatorUI::EmulatorUI(
    Cpu& cpu_ref, Bus& bus_ref, TestSuite& ts_ref, std::string& rom_info_ref,
    bool& paused_ref, bool& step_req_ref, bool& emu_is_running_ref,
    std::function<void(const TestRom&)> load_test_rom_fn,
    std::function<void()> reset_cpu_fn)
    : window_(nullptr), gl_context_(nullptr),
    cpu_(cpu_ref), bus_(bus_ref), test_suite_(ts_ref), current_rom_info_(rom_info_ref),
    is_paused_for_step_(paused_ref), step_requested_(step_req_ref), emulator_is_running_(emu_is_running_ref),
    load_test_rom_callback_(load_test_rom_fn), reset_cpu_callback_(reset_cpu_fn) {
}

EmulatorUI::~EmulatorUI() {
    
}

bool EmulatorUI::initialize() {
    if (!initSdlAndOpenGL()) return false;
    initImGui();
    cpu_state_prev_frame_.capture(cpu_);
    return true;
}

void EmulatorUI::shutdown() {
    cleanupImGui();
    cleanupSdl();
    std::cout << "Emulator UI shutdown." << std::endl;
}

void EmulatorUI::captureCpuStateForDiff() {
    cpu_state_prev_frame_.capture(cpu_);
}

void EmulatorUI::resetDisassemblyViewToPc() {
    disassembly_view_center_address_ = cpu_.pc;
    follow_pc_in_disassembly_ = true;
}

bool EmulatorUI::initSdlAndOpenGL() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_EVENTS) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    window_ = SDL_CreateWindow("GBC Emulator & Debugger", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window_) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit(); return false;
    }
    gl_context_ = SDL_GL_CreateContext(window_);
    if (!gl_context_) {
        std::cerr << "OpenGL context could not be created! SDL Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window_); window_ = nullptr; SDL_Quit(); return false;
    }
    SDL_GL_MakeCurrent(window_, gl_context_);
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        SDL_GL_DeleteContext(gl_context_); gl_context_ = nullptr;
        SDL_DestroyWindow(window_); window_ = nullptr; SDL_Quit(); return false;
    }
    std::cout << "GLAD Initialized. OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    if (SDL_GL_SetSwapInterval(1) < 0) {
        std::cerr << "Warning: Unable to set VSync! SDL Error: " << SDL_GetError() << std::endl;
    }
    std::cout << "SDL and OpenGL context initialized successfully for UI." << std::endl;
    return true;
}

void EmulatorUI::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(window_, gl_context_);
    const char* glsl_version = "#version 330 core";
    ImGui_ImplOpenGL3_Init(glsl_version);
    std::cout << "ImGui initialized successfully for UI." << std::endl;
}

void EmulatorUI::cleanupImGui() {
    if (ImGui::GetCurrentContext()) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        std::cout << "ImGui cleaned up for UI." << std::endl;
    }
}

void EmulatorUI::cleanupSdl() {
    if (gl_context_) { SDL_GL_DeleteContext(gl_context_); gl_context_ = nullptr; }
    if (window_) { SDL_DestroyWindow(window_); window_ = nullptr; }
    if (SDL_WasInit(0) != 0) {
        SDL_Quit();
        std::cout << "SDL cleaned up for UI." << std::endl;
    }
}

void EmulatorUI::processInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        ImGuiIO& io = ImGui::GetIO();
        if (event.type == SDL_QUIT || (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && window_ && event.window.windowID == SDL_GetWindowID(window_))) {
            emulator_is_running_ = false;
        }
    }
}

void EmulatorUI::render() {
    if (!window_ || !ImGui::GetCurrentContext()) return;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    drawCpuRegistersAndStateWindow();
    drawDebugControlsWindow();
    drawDisassemblyContextWindow();
    drawMemoryViewerWindow();
    renderGBCFrame();

    ImGui::Render();

    int display_w, display_h;
    SDL_GetWindowSize(window_, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);

    ImVec4 clear_color = ImVec4(0.1f, 0.1f, 0.12f, 1.00f);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags) {
        SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
        SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
        SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
    }

    SDL_GL_SwapWindow(window_);
}

void EmulatorUI::renderGBCFrame() {
    
}


void EmulatorUI::drawCpuRegistersAndStateWindow() {
    const ImVec4 default_color = ImGui::GetStyleColorVec4(ImGuiCol_Text);
    const ImVec4 changed_color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);

    auto TextDiff16 = [&](const char* label, uint16_t current_val, uint16_t prev_val, const char* fmt = "%s: %s") {
        bool changed = (current_val != prev_val);
        if (changed) ImGui::PushStyleColor(ImGuiCol_Text, changed_color);
        ImGui::Text(fmt, label, formatHex16(current_val).c_str());
        if (changed) ImGui::PopStyleColor();
        };

    auto TextDiffPair = [&](const char* pair_label, uint16_t current_pair, uint16_t prev_pair,
        const char* hi_label, uint8_t current_hi, uint8_t prev_hi,
        const char* lo_label, uint8_t current_lo, uint8_t prev_lo) {
            bool pair_changed = (current_pair != prev_pair);
            bool hi_changed = (current_hi != prev_hi);
            bool lo_changed = (current_lo != prev_lo);

            if (pair_changed) ImGui::PushStyleColor(ImGuiCol_Text, changed_color);
            ImGui::Text("%s: %s (", pair_label, formatHex16(current_pair).c_str());
            if (pair_changed) ImGui::PopStyleColor();
            ImGui::SameLine(0, 0);

            if (hi_changed) ImGui::PushStyleColor(ImGuiCol_Text, changed_color);
            ImGui::Text("%s:%s", hi_label, formatHex8(current_hi).c_str());
            if (hi_changed) ImGui::PopStyleColor();
            ImGui::SameLine(0, 0); ImGui::TextUnformatted(" "); ImGui::SameLine(0, 0);

            if (lo_changed) ImGui::PushStyleColor(ImGuiCol_Text, changed_color);
            ImGui::Text("%s:%s)", lo_label, formatHex8(current_lo).c_str());
            if (lo_changed) ImGui::PopStyleColor();
        };

    ImGui::SetNextWindowSize(ImVec2(550, 280), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("CPU Registers & State")) {
        ImGui::Text("Loaded: %s", current_rom_info_.c_str());
        ImGui::Separator();

        ImGui::BeginChild("RegistersPane", ImVec2(ImGui::GetContentRegionAvail().x * 0.45f, 0), false, ImGuiWindowFlags_NoScrollbar);

        TextDiff16("PC", cpu_.pc, cpu_state_prev_frame_.pc);
        TextDiff16("SP", cpu_.sp, cpu_state_prev_frame_.sp);
        ImGui::Separator();

        
        TextDiffPair("AF", cpu_.af, cpu_state_prev_frame_.af, "A", cpu_.a(), static_cast<uint8_t>(cpu_state_prev_frame_.af >> 8), "F", cpu_.f(), static_cast<uint8_t>(cpu_state_prev_frame_.af & 0xFF));
        TextDiffPair("BC", cpu_.bc, cpu_state_prev_frame_.bc, "B", cpu_.b(), static_cast<uint8_t>(cpu_state_prev_frame_.bc >> 8), "C", cpu_.c(), static_cast<uint8_t>(cpu_state_prev_frame_.bc & 0xFF));
        TextDiffPair("DE", cpu_.de, cpu_state_prev_frame_.de, "D", cpu_.d(), static_cast<uint8_t>(cpu_state_prev_frame_.de >> 8), "E", cpu_.e(), static_cast<uint8_t>(cpu_state_prev_frame_.de & 0xFF));
        TextDiffPair("HL", cpu_.hl, cpu_state_prev_frame_.hl, "H", cpu_.h(), static_cast<uint8_t>(cpu_state_prev_frame_.hl >> 8), "L", cpu_.l(), static_cast<uint8_t>(cpu_state_prev_frame_.hl & 0xFF));

        ImGui::Separator();
        
        uint8_t current_f_reg = cpu_.f();
        uint8_t prev_f_reg = static_cast<uint8_t>(cpu_state_prev_frame_.af & 0xFF);

        bool z_flag_current = (current_f_reg >> Cpu::FLAG_Z_BIT) & 1;
        bool n_flag_current = (current_f_reg >> Cpu::FLAG_N_BIT) & 1;
        bool h_flag_current = (current_f_reg >> Cpu::FLAG_H_BIT) & 1;
        bool c_flag_current = (current_f_reg >> Cpu::FLAG_C_BIT) & 1;

        bool z_flag_prev = (prev_f_reg >> Cpu::FLAG_Z_BIT) & 1;
        bool n_flag_prev = (prev_f_reg >> Cpu::FLAG_N_BIT) & 1;
        bool h_flag_prev = (prev_f_reg >> Cpu::FLAG_H_BIT) & 1;
        bool c_flag_prev = (prev_f_reg >> Cpu::FLAG_C_BIT) & 1;

        bool any_flag_changed = (current_f_reg != prev_f_reg);

        if (any_flag_changed) ImGui::PushStyleColor(ImGuiCol_Text, changed_color);
        ImGui::Text("Flags:");
        if (any_flag_changed) ImGui::PopStyleColor();
        ImGui::SameLine();

        if (z_flag_current != z_flag_prev) ImGui::PushStyleColor(ImGuiCol_Text, changed_color);
        ImGui::Text(" Z:%d", z_flag_current);
        if (z_flag_current != z_flag_prev) ImGui::PopStyleColor();
        ImGui::SameLine();

        if (n_flag_current != n_flag_prev) ImGui::PushStyleColor(ImGuiCol_Text, changed_color);
        ImGui::Text(" N:%d", n_flag_current);
        if (n_flag_current != n_flag_prev) ImGui::PopStyleColor();
        ImGui::SameLine();

        if (h_flag_current != h_flag_prev) ImGui::PushStyleColor(ImGuiCol_Text, changed_color);
        ImGui::Text(" H:%d", h_flag_current);
        if (h_flag_current != h_flag_prev) ImGui::PopStyleColor();
        ImGui::SameLine();

        if (c_flag_current != c_flag_prev) ImGui::PushStyleColor(ImGuiCol_Text, changed_color);
        ImGui::Text(" C:%d", c_flag_current);
        if (c_flag_current != c_flag_prev) ImGui::PopStyleColor();

        ImGui::Separator();
        ImGui::Text("Total Cycles: %llu", (unsigned long long)cpu_.cycles_elapsed_total_);
        if (cpu_.current_instruction_cycles_ != 0 || cpu_state_prev_frame_.last_instr_length > 0) {
            ImGui::Text("Last Op Cycles: %d", cpu_.current_instruction_cycles_);
        }
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("InstructionPane", ImVec2(0, 0), false, 0);
        ImGui::Text("Last Executed @ %s:", formatHex16(cpu_state_prev_frame_.last_instr_pc).c_str());
        std::string last_bytes_str = "";
        for (uint8_t byte_val : cpu_state_prev_frame_.last_instr_bytes_vec) {
            last_bytes_str += formatHex8(byte_val, false) + " ";
        }
        ImGui::Text("Bytes: %s", last_bytes_str.c_str());
        ImGui::Text("Disasm: %s", cpu_state_prev_frame_.last_disassembled_str.c_str());
        if (cpu_state_prev_frame_.last_instr_length > 1) {
            if (cpu_state_prev_frame_.last_instr_length == 2)
                ImGui::Text("Operand: %s", formatHex8(static_cast<uint8_t>(cpu_state_prev_frame_.last_operand)).c_str());
            else if (cpu_state_prev_frame_.last_instr_length == 3)
                ImGui::Text("Operand: %s", formatHex16(cpu_state_prev_frame_.last_operand).c_str());
        }
        ImGui::Separator();
        ImGui::Text("Next to Execute @ %s:", formatHex16(cpu_.pc).c_str());
        uint8_t next_len;
        std::vector<uint8_t> next_bytes_vec;
        std::string next_disasm = cpu_.disassembleInstructionAt(cpu_.pc, next_len, next_bytes_vec);
        std::string next_bytes_display_str = "";
        for (uint8_t byte_val : next_bytes_vec) {
            next_bytes_display_str += formatHex8(byte_val, false) + " ";
        }
        ImGui::Text("Bytes: %s", next_bytes_display_str.c_str());
        ImGui::Text("Disasm: %s", next_disasm.c_str());
        ImGui::EndChild();
    }
    ImGui::End();
}

void EmulatorUI::drawDebugControlsWindow() {
    ImGui::SetNextWindowSize(ImVec2(450, 150), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(570, 10), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Debug Controls")) {
        ImGui::Text("State: %s", is_paused_for_step_ ? "Paused" : "Running");
        ImGui::SameLine(); ImGui::Text("(Step Req: %s)", step_requested_ ? "YES" : "no ");
        if (is_paused_for_step_) {
            if (ImGui::Button("Step Into")) { step_requested_ = true; }
            ImGui::SameLine();
            if (ImGui::Button("Run")) { is_paused_for_step_ = false; step_requested_ = false; }
        }
        else {
            if (ImGui::Button("Pause")) { is_paused_for_step_ = true; step_requested_ = false; }
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset CPU")) {
            if (reset_cpu_callback_) reset_cpu_callback_();
        }
        ImGui::Separator();
        ImGui::Text("Load Test ROM:");
        const auto& all_tests = test_suite_.getAllTests();
        static int selected_test_idx = 0;
        if (!all_tests.empty()) {
            if (selected_test_idx >= static_cast<int>(all_tests.size())) selected_test_idx = 0;
            if (ImGui::BeginCombo("Select Test", all_tests[selected_test_idx].name.c_str())) {
                for (int n = 0; n < static_cast<int>(all_tests.size()); n++) {
                    const bool is_selected = (selected_test_idx == n);
                    if (ImGui::Selectable(all_tests[n].name.c_str(), is_selected)) { selected_test_idx = n; }
                    if (is_selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            if (ImGui::Button("Load Selected Test")) {
                if (selected_test_idx >= 0 && selected_test_idx < static_cast<int>(all_tests.size())) {
                    if (load_test_rom_callback_) load_test_rom_callback_(all_tests[selected_test_idx]);
                }
            }
        }
        else { ImGui::Text("No tests available in TestSuite."); }
    }
    ImGui::End();
}

void EmulatorUI::drawDisassemblyContextWindow() {
    const ImVec4 pc_highlight_color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);

    ImGui::SetNextWindowSize(ImVec2(550, 300), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(10, 300), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Disassembly Context")) {
        ImGui::Checkbox("Follow PC", &follow_pc_in_disassembly_);
        ImGui::SameLine();
        static char addr_buf[5] = "0100";
        ImGui::PushItemWidth(60);
        if (ImGui::InputText("Goto Address", addr_buf, 5, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue)) {
            unsigned int new_addr_uint;
            if (sscanf(addr_buf, "%x", &new_addr_uint) == 1) {
                disassembly_view_center_address_ = static_cast<uint16_t>(new_addr_uint);
                follow_pc_in_disassembly_ = false;
            }
        }
        ImGui::PopItemWidth();

        if (follow_pc_in_disassembly_) {
            int pc_val = cpu_.pc;
            int center_val = disassembly_view_center_address_;
            int diff = pc_val - center_val;
            if (diff < -15 || diff > 15) {
                disassembly_view_center_address_ = cpu_.pc;
            }
        }

        ImGui::Separator();
        ImGui::BeginChild("DisassemblyScroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        const int num_lines_display = 25;
        const int lines_before_target_ideal = 10;
        uint16_t current_disasm_addr = disassembly_view_center_address_;

        uint16_t temp_addr = disassembly_view_center_address_;
        std::vector<uint16_t> prev_addrs_stack;
        for (int i = 0; i < lines_before_target_ideal + 5; ++i) {
            bool found_an_earlier_instruction = false;
            for (uint16_t look_back_offset = 1; look_back_offset <= 3; ++look_back_offset) {
                if (temp_addr < look_back_offset) break;
                uint16_t prev_potential_addr = temp_addr - look_back_offset;

                uint8_t L; std::vector<uint8_t> B;
                cpu_.disassembleInstructionAt(prev_potential_addr, L, B);
                if (L == 0) L = 1;

                if (prev_potential_addr + L == temp_addr) {
                    prev_addrs_stack.push_back(prev_potential_addr);
                    temp_addr = prev_potential_addr;
                    found_an_earlier_instruction = true;
                    break;
                }
            }
            if (!found_an_earlier_instruction || prev_addrs_stack.size() >= lines_before_target_ideal) {
                break;
            }
        }
        if (prev_addrs_stack.size() >= lines_before_target_ideal && !prev_addrs_stack.empty()) {
            current_disasm_addr = prev_addrs_stack.back();
        }
        else if (!prev_addrs_stack.empty() && prev_addrs_stack.back() < disassembly_view_center_address_) {
            current_disasm_addr = prev_addrs_stack.back();
        }
        else {
            current_disasm_addr = (disassembly_view_center_address_ > lines_before_target_ideal / 2u) ? (disassembly_view_center_address_ - lines_before_target_ideal / 2u) : 0u;
            current_disasm_addr = std::min(current_disasm_addr, disassembly_view_center_address_);
        }


        bool pc_line_visible_and_followed = false;

        for (int line_count = 0; line_count < num_lines_display; ++line_count) {
            if (current_disasm_addr > 0xFFFF - 3 && current_disasm_addr <= 0xFFFF) {
            }
            else if (line_count > 5 && current_disasm_addr > cpu_.pc + 60 && cpu_.pc < 0xFFA0) {
            }

            uint8_t len = 0;
            std::vector<uint8_t> current_bytes_vec;
            std::string dis = "??";
            if (current_disasm_addr <= 0xFFFF) {
                dis = cpu_.disassembleInstructionAt(current_disasm_addr, len, current_bytes_vec);
            }
            else {
                break;
            }

            std::string bytes_str_loop = "";
            for (size_t k = 0; k < current_bytes_vec.size() && k < 4; ++k) {
                bytes_str_loop += formatHex8(current_bytes_vec[k], false) + " ";
            }
            while (bytes_str_loop.length() < 12) bytes_str_loop += " ";

            bool is_pc_line = (current_disasm_addr == cpu_.pc);
            if (is_pc_line) {
                ImGui::PushStyleColor(ImGuiCol_Text, pc_highlight_color);
                if (follow_pc_in_disassembly_) {
                    pc_line_visible_and_followed = true;
                }
            }

            ImGui::Text("%s: %s%s", formatHex16(current_disasm_addr).c_str(), bytes_str_loop.c_str(), dis.c_str());

            if (is_pc_line) {
                ImGui::PopStyleColor();
                if (follow_pc_in_disassembly_) ImGui::SetScrollHereY(0.4f);
            }

            if (len == 0) len = 1;

            uint32_t next_addr_check = static_cast<uint32_t>(current_disasm_addr) + len;
            if (next_addr_check > 0xFFFF) break;
            current_disasm_addr = static_cast<uint16_t>(next_addr_check);
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

void EmulatorUI::drawMemoryViewerWindow() {
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver); 
    ImGui::SetNextWindowPos(ImVec2(570, 170), ImGuiCond_FirstUseEver); 

    if (ImGui::Begin("Memory Viewer")) {
        
        ImGui::PushItemWidth(70); 
        if (ImGui::InputText("Address", memory_editor_addr_input_buf_, 5,
            ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue)) {
            unsigned int new_addr_uint;
            if (sscanf(memory_editor_addr_input_buf_, "%x", &new_addr_uint) == 1) {
                memory_editor_address_ = static_cast<uint16_t>(new_addr_uint);
            }
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::ArrowButton("##mem_prev_page", ImGuiDir_Left)) {
            if (memory_editor_address_ >= static_cast<uint16_t>(memory_editor_bytes_per_row_ * memory_editor_num_rows_)) {
                memory_editor_address_ -= static_cast<uint16_t>(memory_editor_bytes_per_row_ * memory_editor_num_rows_);
            }
            else {
                memory_editor_address_ = 0;
            }
            sprintf(memory_editor_addr_input_buf_, "%04X", memory_editor_address_);
        }
        ImGui::SameLine();
        if (ImGui::ArrowButton("##mem_next_page", ImGuiDir_Right)) {
            uint32_t new_addr = static_cast<uint32_t>(memory_editor_address_) + (memory_editor_bytes_per_row_ * memory_editor_num_rows_);
            if (new_addr <= 0xFFFF) {
                memory_editor_address_ = static_cast<uint16_t>(new_addr);
            }
            else {
                
                
                uint32_t max_start_addr = 0xFFFF - (memory_editor_bytes_per_row_ * memory_editor_num_rows_) + 1;
                max_start_addr = std::min(max_start_addr, (uint32_t)0xFFFF); 
                memory_editor_address_ = static_cast<uint16_t>(max_start_addr);
            }
            sprintf(memory_editor_addr_input_buf_, "%04X", memory_editor_address_);
        }
        ImGui::SameLine();
        ImGui::Text(" (SP: %s, PC: %s)", formatHex16(cpu_.sp).c_str(), formatHex16(cpu_.pc).c_str());


        ImGui::Separator();

        
        
        ImGui::BeginChild("MemoryViewScroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        ImGuiListClipper clipper;
        
        
        clipper.Begin(0xFFFF / memory_editor_bytes_per_row_ + 1, ImGui::GetTextLineHeightWithSpacing());

        
        uint16_t current_row_addr = memory_editor_address_;

        for (int row = 0; row < memory_editor_num_rows_; ++row) {
            if (static_cast<uint32_t>(current_row_addr) + row * memory_editor_bytes_per_row_ > 0xFFFF && row > 0) {
                
                
                break;
            }

            
            ImGui::Text("%s: ", formatHex16(current_row_addr + row * memory_editor_bytes_per_row_).c_str());
            ImGui::SameLine();

            
            std::string ascii_representation = " | "; 
            for (int col = 0; col < memory_editor_bytes_per_row_; ++col) {
                uint32_t current_byte_addr_long = static_cast<uint32_t>(current_row_addr) +
                    row * memory_editor_bytes_per_row_ + col;

                if (current_byte_addr_long > 0xFFFF) {
                    ImGui::TextUnformatted(".."); 
                }
                else {
                    uint16_t current_byte_addr = static_cast<uint16_t>(current_byte_addr_long);
                    uint8_t byte_val = bus_.read(current_byte_addr);

                    
                    bool is_sp = (current_byte_addr == cpu_.sp);
                    bool is_pc = (current_byte_addr == cpu_.pc);
                    if (is_sp && is_pc) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.5f, 1.0f, 1.0f)); 
                    else if (is_sp) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 1.0f, 1.0f));    
                    else if (is_pc) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));    

                    ImGui::TextUnformatted(formatHex8(byte_val, false).c_str());

                    if (is_sp || is_pc) ImGui::PopStyleColor();

                    
                    if (byte_val >= 32 && byte_val <= 126) { 
                        ascii_representation += static_cast<char>(byte_val);
                    }
                    else {
                        ascii_representation += '.'; 
                    }
                }
                if (col < memory_editor_bytes_per_row_ - 1) {
                    ImGui::SameLine(0, ImGui::GetStyle().ItemSpacing.x / 2); 
                }
            }

            
            ImGui::SameLine();
            ImGui::TextUnformatted(ascii_representation.c_str());

            
            if (static_cast<uint32_t>(current_row_addr) + (row + 1) * memory_editor_bytes_per_row_ > 0xFFFF &&
                (current_row_addr % memory_editor_bytes_per_row_ != 0 || row * memory_editor_bytes_per_row_ < 0xFFFF)) {
                
                
                break;
            }
        }
        

        
        if (ImGui::IsWindowHovered() && ImGui::GetIO().MouseWheel != 0) {
            int scroll_amount_lines = static_cast<int>(-ImGui::GetIO().MouseWheel); 
            int scroll_amount_bytes = scroll_amount_lines * memory_editor_bytes_per_row_;

            int32_t new_addr_signed = static_cast<int32_t>(memory_editor_address_) + scroll_amount_bytes;

            if (new_addr_signed < 0) {
                memory_editor_address_ = 0;
            }
            else if (new_addr_signed > 0xFFFF) {
                
                uint32_t max_start_addr = 0xFFFF - std::min(memory_editor_bytes_per_row_ * (memory_editor_num_rows_ - 1), 0xFFFF);
                memory_editor_address_ = static_cast<uint16_t>(max_start_addr);

            }
            else {
                memory_editor_address_ = static_cast<uint16_t>(new_addr_signed);
            }
            sprintf(memory_editor_addr_input_buf_, "%04X", memory_editor_address_); 
        }


        ImGui::EndChild(); 
    }
    ImGui::End(); 
}