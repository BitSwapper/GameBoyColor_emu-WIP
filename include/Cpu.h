#ifndef CPU_H
#define CPU_H

#include <cstdint>
#include <string>
#include <vector>
#include <memory> 

class Bus;
class Instruction;
#include "Utils.h" 

class Cpu {
public:
    
    uint16_t af, bc, de, hl;
    uint16_t sp, pc;

    
    uint8_t a() const { return static_cast<uint8_t>(af >> 8); }
    void set_a(uint8_t val) { af = (static_cast<uint16_t>(val) << 8) | (af & 0x00FF); }
    uint8_t f() const { return static_cast<uint8_t>(af & 0x00FF); }
    void set_f(uint8_t val) { af = (af & 0xFF00) | (val & 0xF0); }

    uint8_t b() const { return static_cast<uint8_t>(bc >> 8); }
    void set_b(uint8_t val) { bc = (static_cast<uint16_t>(val) << 8) | (bc & 0x00FF); }
    uint8_t c() const { return static_cast<uint8_t>(bc & 0x00FF); }
    void set_c(uint8_t val) { bc = (bc & 0xFF00) | val; }

    uint8_t d() const { return static_cast<uint8_t>(de >> 8); }
    void set_d(uint8_t val) { de = (static_cast<uint16_t>(val) << 8) | (de & 0x00FF); }
    uint8_t e() const { return static_cast<uint8_t>(de & 0x00FF); }
    void set_e(uint8_t val) { de = (de & 0xFF00) | val; }

    uint8_t h() const { return static_cast<uint8_t>(hl >> 8); }
    void set_h(uint8_t val) { hl = (static_cast<uint16_t>(val) << 8) | (hl & 0x00FF); }
    uint8_t l() const { return static_cast<uint8_t>(hl & 0x00FF); }
    void set_l(uint8_t val) { hl = (hl & 0xFF00) | val; }

    
    static const int FLAG_Z_BIT = 7;
    static const int FLAG_N_BIT = 6;
    static const int FLAG_H_BIT = 5;
    static const int FLAG_C_BIT = 4;

    
    bool getFlagZ() const { return (f() >> FLAG_Z_BIT) & 1; }
    void setFlagZ(bool val) { uint8_t temp_f = f(); if (val) temp_f |= (1 << FLAG_Z_BIT); else temp_f &= ~(1 << FLAG_Z_BIT); set_f(temp_f); }
    bool getFlagN() const { return (f() >> FLAG_N_BIT) & 1; }
    void setFlagN(bool val) { uint8_t temp_f = f(); if (val) temp_f |= (1 << FLAG_N_BIT); else temp_f &= ~(1 << FLAG_N_BIT); set_f(temp_f); }
    bool getFlagH() const { return (f() >> FLAG_H_BIT) & 1; }
    void setFlagH(bool val) { uint8_t temp_f = f(); if (val) temp_f |= (1 << FLAG_H_BIT); else temp_f &= ~(1 << FLAG_H_BIT); set_f(temp_f); }
    bool getFlagC() const { return (f() >> FLAG_C_BIT) & 1; }
    void setFlagC(bool val) { uint8_t temp_f = f(); if (val) temp_f |= (1 << FLAG_C_BIT); else temp_f &= ~(1 << FLAG_C_BIT); set_f(temp_f); }

    
    uint16_t debug_last_instr_pc_;
    uint8_t  debug_last_opcode_;
    uint16_t debug_last_operand_;
    uint8_t  debug_last_instr_length_;
    std::string debug_last_disassembled_;
    std::vector<uint8_t> debug_last_instr_bytes_;

    uint64_t cycles_elapsed_total_;
    uint8_t  current_instruction_cycles_;

    Cpu();
    ~Cpu();

    void connectBus(const std::shared_ptr<Bus>& bus_ptr);
    void reset();
    void step();

    uint8_t busRead(uint16_t address);
    void busWrite(uint16_t address, uint8_t data);

    Instruction* getCbInstruction(uint8_t cb_opcode);
    std::string disassembleInstructionAt(uint16_t address, uint8_t& out_length, std::vector<uint8_t>& out_bytes);
    
    void updateFlags_INC8(uint8_t old_val, uint8_t new_val);
    
    void updateFlags_DEC8(uint8_t old_val, uint8_t new_val);
    
    void updateFlags_ADD8(uint8_t val_a, uint8_t val_b, uint16_t result_wide);
    
    void updateFlags_SUB8(uint8_t val_a, uint8_t val_b, uint8_t result_byte);
    
    void updateFlags_LOGIC8(uint8_t result_val, bool h_flag_val); 

private:
    void initializeInstructionTables();

    std::shared_ptr<Bus> bus_;
    std::vector<std::unique_ptr<Instruction>> instruction_table_;
    std::vector<std::unique_ptr<Instruction>> cb_instruction_table_;
};

#endif 