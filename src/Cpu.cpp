#include "Cpu.h"
#include "Bus.h"
#include "InvalidInstruction.h"
#include "Opcodes.h" 
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>

Cpu::Cpu() {
    instruction_table_.resize(0x100);
    cb_instruction_table_.resize(0x100);
    initializeInstructionTables();
    reset();
}

Cpu::~Cpu() {
}

void Cpu::connectBus(const std::shared_ptr<Bus>& bus_ptr) {
    bus_ = bus_ptr;
}

void Cpu::reset() {
    af = 0x01B0; bc = 0x0013; de = 0x00D8; hl = 0x014D;
    sp = 0xFFFE; pc = 0x0100;

    cycles_elapsed_total_ = 0;
    current_instruction_cycles_ = 0;
    debug_last_instr_pc_ = 0;
    debug_last_opcode_ = 0;
    debug_last_operand_ = 0;
    debug_last_instr_length_ = 0;
    debug_last_disassembled_ = "RESET";
    debug_last_instr_bytes_.clear();
}

uint8_t Cpu::busRead(uint16_t address) {
    if (!bus_) {
        std::cerr << "FATAL: CPU busRead with no bus connected!" << std::endl;
        return 0xFF;
    }
    return bus_->read(address);
}

void Cpu::busWrite(uint16_t address, uint8_t data) {
    if (!bus_) {
        std::cerr << "FATAL: CPU busWrite with no bus connected!" << std::endl;
        return;
    }
    bus_->write(address, data);
}

Instruction* Cpu::getCbInstruction(uint8_t cb_opcode) {
    if (cb_opcode < cb_instruction_table_.size() && cb_instruction_table_[cb_opcode]) {
        return cb_instruction_table_[cb_opcode].get();
    }
    std::cerr << "CRITICAL: CB Opcode " << formatHex8(cb_opcode) << " out of cb_instruction_table_ bounds or table not initialized." << std::endl;
    static InvalidInstruction static_invalid_cb_handler(cb_opcode);
    return &static_invalid_cb_handler;
}

void Cpu::initializeInstructionTables() {
    
    for (int i = 0; i < 0x100; ++i) {
        instruction_table_[i] = std::make_unique<InvalidInstruction>(static_cast<uint8_t>(i));
        cb_instruction_table_[i] = std::make_unique<InvalidInstruction>(static_cast<uint8_t>(i));
    }


    instruction_table_[0x06] = std::make_unique<Instr_LD_R_D8>(0); instruction_table_[0x0E] = std::make_unique<Instr_LD_R_D8>(1);
    instruction_table_[0x16] = std::make_unique<Instr_LD_R_D8>(2); instruction_table_[0x1E] = std::make_unique<Instr_LD_R_D8>(3);
    instruction_table_[0x26] = std::make_unique<Instr_LD_R_D8>(4); instruction_table_[0x2E] = std::make_unique<Instr_LD_R_D8>(5);
    instruction_table_[0x3E] = std::make_unique<Instr_LD_R_D8>(7);
    
    instruction_table_[0x36] = std::make_unique<Instr_LD_MHL_D8>();
    
    instruction_table_[0x70] = std::make_unique<Instr_LD_MHL_R>(0); instruction_table_[0x71] = std::make_unique<Instr_LD_MHL_R>(1);
    instruction_table_[0x72] = std::make_unique<Instr_LD_MHL_R>(2); instruction_table_[0x73] = std::make_unique<Instr_LD_MHL_R>(3);
    instruction_table_[0x74] = std::make_unique<Instr_LD_MHL_R>(4); instruction_table_[0x75] = std::make_unique<Instr_LD_MHL_R>(5);
    instruction_table_[0x77] = std::make_unique<Instr_LD_MHL_R>(7);
    
    instruction_table_[0x01] = std::make_unique<Instr_LD_RR_D16>(0); instruction_table_[0x11] = std::make_unique<Instr_LD_RR_D16>(1);
    instruction_table_[0x21] = std::make_unique<Instr_LD_RR_D16>(2); instruction_table_[0x31] = std::make_unique<Instr_LD_RR_D16>(3);
    
    instruction_table_[0x04] = std::make_unique<Instr_INC_R>(0); instruction_table_[0x0C] = std::make_unique<Instr_INC_R>(1);
    instruction_table_[0x14] = std::make_unique<Instr_INC_R>(2); instruction_table_[0x1C] = std::make_unique<Instr_INC_R>(3);
    instruction_table_[0x24] = std::make_unique<Instr_INC_R>(4); instruction_table_[0x2C] = std::make_unique<Instr_INC_R>(5);
    instruction_table_[0x3C] = std::make_unique<Instr_INC_R>(7);
    
    instruction_table_[0x34] = std::make_unique<Instr_INC_MHL>();
    
    instruction_table_[0x05] = std::make_unique<Instr_DEC_R>(0); instruction_table_[0x0D] = std::make_unique<Instr_DEC_R>(1);
    instruction_table_[0x15] = std::make_unique<Instr_DEC_R>(2); instruction_table_[0x1D] = std::make_unique<Instr_DEC_R>(3);
    instruction_table_[0x25] = std::make_unique<Instr_DEC_R>(4); instruction_table_[0x2D] = std::make_unique<Instr_DEC_R>(5);
    instruction_table_[0x3D] = std::make_unique<Instr_DEC_R>(7);
    
    for (int i = 0; i < 8; ++i) { instruction_table_[0x80 + i] = std::make_unique<Instr_ADD_A_R>(i); }
    
    for (int i = 0; i < 8; ++i) { instruction_table_[0x90 + i] = std::make_unique<Instr_SUB_A_R>(i); }
    
    instruction_table_[0xAF] = std::make_unique<Instr_XOR_A>(); 
    
    instruction_table_[0xC3] = std::make_unique<Instr_JP_A16>();
    
    instruction_table_[0x00] = std::make_unique<Instr_NOP>();
    instruction_table_[0x76] = std::make_unique<Instr_HALT>();
    
    instruction_table_[0xCB] = std::make_unique<Instr_CB_PREFIX>();

    
    for (uint8_t opcode = 0x40; opcode <= 0x7F; ++opcode) {
        if (opcode == 0x76) continue; 

        uint8_t dest_idx = (opcode & 0b00111000) >> 3;
        uint8_t src_idx = (opcode & 0b00000111);

        if (dest_idx == 6) { 
            continue;
        }
        
        instruction_table_[opcode] = std::make_unique<Instr_LD_R_R>(dest_idx, src_idx);
    }
    
    instruction_table_[0x0A] = std::make_unique<Instr_LD_A_MRR>(0); 
    
    instruction_table_[0x1A] = std::make_unique<Instr_LD_A_MRR>(1); 

    instruction_table_[0x02] = std::make_unique<Instr_LD_MRR_A>(0); 
    
    instruction_table_[0x12] = std::make_unique<Instr_LD_MRR_A>(1); 
        
    instruction_table_[0xFA] = std::make_unique<Instr_LD_A_MA16>();
    
    instruction_table_[0xEA] = std::make_unique<Instr_LD_MA16_A>();
}


void Cpu::updateFlags_INC8(uint8_t old_val, uint8_t new_val) {
    setFlagZ(new_val == 0);
    setFlagN(false);
    setFlagH((old_val & 0x0F) == 0x0F); 
}

void Cpu::updateFlags_DEC8(uint8_t old_val, uint8_t new_val) {
    setFlagZ(new_val == 0);
    setFlagN(true);
    setFlagH((old_val & 0x0F) == 0x00); 
}

void Cpu::updateFlags_ADD8(uint8_t val_a, uint8_t val_b, uint16_t result_wide) {
    uint8_t result_byte = static_cast<uint8_t>(result_wide);
    setFlagZ(result_byte == 0);
    setFlagN(false);
    setFlagH(((val_a & 0x0F) + (val_b & 0x0F)) > 0x0F);
    setFlagC(result_wide > 0xFF);
}

void Cpu::updateFlags_SUB8(uint8_t val_a, uint8_t val_b, uint8_t result_byte) {
    setFlagZ(result_byte == 0);
    setFlagN(true);
    setFlagH((val_a & 0x0F) < (val_b & 0x0F));
    setFlagC(val_a < val_b);
}

void Cpu::updateFlags_LOGIC8(uint8_t result_val, bool h_flag_val) {
    setFlagZ(result_val == 0);
    setFlagN(false);
    setFlagH(h_flag_val); 
    setFlagC(false);
}

void Cpu::step() {
    if (!bus_) {
        std::cerr << "CPU Step: No bus connected!" << std::endl;
        return;
    }

    debug_last_instr_pc_ = pc; 

    uint8_t opcode = busRead(pc);
    pc++; 

    debug_last_opcode_ = opcode;

    Instruction* instr = nullptr;
    if (opcode < instruction_table_.size() && instruction_table_[opcode]) {
        instr = instruction_table_[opcode].get();
    }
    else {
        std::cerr << "CRITICAL: Opcode " << formatHex8(opcode) << " has no entry in instruction_table_." << std::endl;
        static InvalidInstruction static_invalid_handler(opcode); 
        instr = &static_invalid_handler;
    }
    
    
    instr->execute(*this);    
    
    uint8_t disasm_temp_len; 
    debug_last_instr_bytes_.clear();
    debug_last_disassembled_ = disassembleInstructionAt(debug_last_instr_pc_,
        disasm_temp_len,
        debug_last_instr_bytes_);

    cycles_elapsed_total_ += current_instruction_cycles_;
  
}

std::string Cpu::disassembleInstructionAt(uint16_t address, uint8_t& out_length, std::vector<uint8_t>& out_bytes) {
    
    out_bytes.clear();
    out_length = 0;

    if (!bus_) { return "ERR:NO_BUS"; }

    uint8_t opcode_at_addr = busRead(address);
    Instruction* instr_to_disassemble = nullptr;

    if (opcode_at_addr < instruction_table_.size() && instruction_table_[opcode_at_addr]) {
        instr_to_disassemble = instruction_table_[opcode_at_addr].get();
    }
    else {
        out_bytes.push_back(opcode_at_addr);
        out_length = 1;
        return "DB " + formatHex8(opcode_at_addr) + " ; Unknown Opcode";
    }
    
    std::string disasm_string = instr_to_disassemble->disassemble(*this, address, out_bytes);
    out_length = static_cast<uint8_t>(out_bytes.size());

    if (out_length == 0 && !(disasm_string.rfind("ERR:", 0) == 0) && !(instr_to_disassemble->isInvalid())) {
        
        
        std::cerr << "Warning: Disassemble for opcode " << formatHex8(opcode_at_addr)
            << " did not populate out_bytes. Defaulting to 1 byte." << std::endl;
        out_bytes.push_back(opcode_at_addr); 
        out_length = 1;
    }
    return disasm_string;
}