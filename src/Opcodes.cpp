#include "Opcodes.h"
#include "Cpu.h"
#include "Utils.h" 
#include <sstream>
#include <iostream> 
#include <iomanip>


namespace {
    uint8_t fetch_d8_operand(Cpu& cpu) {
        uint8_t value = cpu.busRead(cpu.pc);
        cpu.pc++;
        return value;
    }
    uint16_t fetch_d16_operand(Cpu& cpu) {
        uint8_t lo = cpu.busRead(cpu.pc);
        cpu.pc++;
        uint8_t hi = cpu.busRead(cpu.pc);
        cpu.pc++;
        return (static_cast<uint16_t>(hi) << 8) | lo;
    }
    const char* reg_name_lut[] = { "B", "C", "D", "E", "H", "L", "(HL)", "A" };
    const char* get_reg_name(int r_idx) {
        if (r_idx >= 0 && r_idx < 8) return reg_name_lut[r_idx];
        return "??R";
    }
    const char* rp_name_lut[] = { "BC", "DE", "HL", "SP" };
    const char* get_rp_name(int rp_idx) {
        if (rp_idx >= 0 && rp_idx < 4) return rp_name_lut[rp_idx];
        return "??RP";
    }

    
    uint8_t get_reg_value(Cpu& cpu, uint8_t reg_idx) {
        switch (reg_idx) {
            case 0: return cpu.b();
            case 1: return cpu.c();
            case 2: return cpu.d();
            case 3: return cpu.e();
            case 4: return cpu.h();
            case 5: return cpu.l();
            
            case 7: return cpu.a();
            default: return 0xFF; 
        }
    }

    
    void set_reg_value(Cpu& cpu, uint8_t reg_idx, uint8_t value) {
        switch (reg_idx) {
            case 0: cpu.set_b(value); break;
            case 1: cpu.set_c(value); break;
            case 2: cpu.set_d(value); break;
            case 3: cpu.set_e(value); break;
            case 4: cpu.set_h(value); break;
            case 5: cpu.set_l(value); break;
            
            case 7: cpu.set_a(value); break;
            
        }
    }
} 





void Instr_NOP::execute(Cpu& cpu) {
    cpu.current_instruction_cycles_ = 4;
    cpu.debug_last_instr_length_ = 1; 
    cpu.debug_last_operand_ = 0;      
}
std::string Instr_NOP::disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) {
    instruction_bytes.push_back(cpu.busRead(pc_at_opcode)); 
    return "NOP";
}

Instr_LD_RR_D16::Instr_LD_RR_D16(uint8_t rp) : rp_index_(rp) {}
void Instr_LD_RR_D16::execute(Cpu& cpu) {
    uint16_t value = fetch_d16_operand(cpu); 
    switch (rp_index_) {
    case 0: cpu.bc = value; break;
    case 1: cpu.de = value; break;
    case 2: cpu.hl = value; break;
    case 3: cpu.sp = value; break;
    }
    cpu.current_instruction_cycles_ = 12;
    cpu.debug_last_instr_length_ = 3; 
    cpu.debug_last_operand_ = value;
}
std::string Instr_LD_RR_D16::disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) {
    instruction_bytes.push_back(cpu.busRead(pc_at_opcode)); 
    uint8_t lo = cpu.busRead(pc_at_opcode + 1);
    uint8_t hi = cpu.busRead(pc_at_opcode + 2);
    instruction_bytes.push_back(lo);
    instruction_bytes.push_back(hi);
    uint16_t d16_val = (static_cast<uint16_t>(hi) << 8) | lo;
    std::ostringstream oss;
    oss << "LD " << get_rp_name(rp_index_) << ", " << formatHex16(d16_val);
    return oss.str();
}

Instr_INC_R::Instr_INC_R(uint8_t reg) : reg_index_(reg) {}
void Instr_INC_R::execute(Cpu& cpu) {
    uint8_t old_val;
    uint8_t new_val;
    switch (reg_index_) {
    case 0: old_val = cpu.b(); cpu.set_b(old_val + 1); new_val = cpu.b(); break;
    case 1: old_val = cpu.c(); cpu.set_c(old_val + 1); new_val = cpu.c(); break;
    case 2: old_val = cpu.d(); cpu.set_d(old_val + 1); new_val = cpu.d(); break;
    case 3: old_val = cpu.e(); cpu.set_e(old_val + 1); new_val = cpu.e(); break;
    case 4: old_val = cpu.h(); cpu.set_h(old_val + 1); new_val = cpu.h(); break;
    case 5: old_val = cpu.l(); cpu.set_l(old_val + 1); new_val = cpu.l(); break;
    case 7: old_val = cpu.a(); cpu.set_a(old_val + 1); new_val = cpu.a(); break;
    default:
        std::cerr << "Instr_INC_R: Invalid reg_index " << (int)reg_index_ << std::endl;
        cpu.current_instruction_cycles_ = 4; cpu.debug_last_instr_length_ = 1; cpu.debug_last_operand_ = 0; return;
    }
    cpu.updateFlags_INC8(old_val, new_val);
    cpu.current_instruction_cycles_ = 4;
    cpu.debug_last_instr_length_ = 1;
    cpu.debug_last_operand_ = 0;
}
std::string Instr_INC_R::disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) {
    instruction_bytes.push_back(cpu.busRead(pc_at_opcode));
    std::ostringstream oss;
    oss << "INC " << get_reg_name(reg_index_);
    return oss.str();
}

Instr_LD_R_D8::Instr_LD_R_D8(uint8_t reg) : reg_index_(reg) {}
void Instr_LD_R_D8::execute(Cpu& cpu) {
    uint8_t value = fetch_d8_operand(cpu); 
    switch (reg_index_) {
    case 0: cpu.set_b(value); break;
    case 1: cpu.set_c(value); break;
    case 2: cpu.set_d(value); break;
    case 3: cpu.set_e(value); break;
    case 4: cpu.set_h(value); break;
    case 5: cpu.set_l(value); break;
    case 7: cpu.set_a(value); break;
    default:
        std::cerr << "Instr_LD_R_D8: Invalid reg_index " << (int)reg_index_ << std::endl;
        cpu.current_instruction_cycles_ = 4; cpu.debug_last_instr_length_ = 2; cpu.debug_last_operand_ = value; return; 
    }
    cpu.current_instruction_cycles_ = 8;
    cpu.debug_last_instr_length_ = 2; 
    cpu.debug_last_operand_ = value;
}
std::string Instr_LD_R_D8::disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) {
    instruction_bytes.push_back(cpu.busRead(pc_at_opcode));
    uint8_t d8_val = cpu.busRead(pc_at_opcode + 1);
    instruction_bytes.push_back(d8_val);
    std::ostringstream oss;
    oss << "LD " << get_reg_name(reg_index_) << ", " << formatHex8(d8_val);
    return oss.str();
}

Instr_ADD_A_R::Instr_ADD_A_R(uint8_t reg) : reg_index_(reg) {}
void Instr_ADD_A_R::execute(Cpu& cpu) {
    uint8_t value_to_add;
    if (reg_index_ == 6) { 
        value_to_add = cpu.busRead(cpu.hl);
        cpu.current_instruction_cycles_ = 8;
    } else {
        value_to_add = get_reg_value(cpu, reg_index_); 
        cpu.current_instruction_cycles_ = 4;
    }
    uint8_t current_a = cpu.a();
    uint16_t result_wide = static_cast<uint16_t>(current_a) + value_to_add;
    cpu.set_a(static_cast<uint8_t>(result_wide));
    cpu.updateFlags_ADD8(current_a, value_to_add, result_wide);
    cpu.debug_last_instr_length_ = 1;
    cpu.debug_last_operand_ = 0;
}
std::string Instr_ADD_A_R::disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) {
    instruction_bytes.push_back(cpu.busRead(pc_at_opcode));
    std::ostringstream oss;
    oss << "ADD A, " << get_reg_name(reg_index_);
    return oss.str();
}

Instr_SUB_A_R::Instr_SUB_A_R(uint8_t reg) : reg_index_(reg) {}
void Instr_SUB_A_R::execute(Cpu& cpu) {
    uint8_t value_to_sub;
    if (reg_index_ == 6) { 
        value_to_sub = cpu.busRead(cpu.hl);
        cpu.current_instruction_cycles_ = 8;
    } else {
        value_to_sub = get_reg_value(cpu, reg_index_); 
        cpu.current_instruction_cycles_ = 4;
    }
    uint8_t old_a = cpu.a();
    uint8_t result_byte = old_a - value_to_sub;
    cpu.set_a(result_byte);
    cpu.updateFlags_SUB8(old_a, value_to_sub, result_byte);
    cpu.debug_last_instr_length_ = 1;
    cpu.debug_last_operand_ = 0;
}
std::string Instr_SUB_A_R::disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) {
    instruction_bytes.push_back(cpu.busRead(pc_at_opcode));
    std::ostringstream oss;
    oss << "SUB A, " << get_reg_name(reg_index_);
    return oss.str();
}

void Instr_XOR_A::execute(Cpu& cpu) {
    cpu.set_a(0);
    cpu.updateFlags_LOGIC8(0, false); 
    cpu.current_instruction_cycles_ = 4;
    cpu.debug_last_instr_length_ = 1;
    cpu.debug_last_operand_ = 0;
}
std::string Instr_XOR_A::disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) {
    instruction_bytes.push_back(cpu.busRead(pc_at_opcode));
    return "XOR A";
}

void Instr_JP_A16::execute(Cpu& cpu) {
    uint16_t target_addr = fetch_d16_operand(cpu); 
    cpu.pc = target_addr; 
    cpu.current_instruction_cycles_ = 16;
    cpu.debug_last_instr_length_ = 3; 
    cpu.debug_last_operand_ = target_addr;
}
std::string Instr_JP_A16::disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) {
    instruction_bytes.push_back(cpu.busRead(pc_at_opcode));
    uint8_t lo = cpu.busRead(pc_at_opcode + 1);
    uint8_t hi = cpu.busRead(pc_at_opcode + 2);
    instruction_bytes.push_back(lo);
    instruction_bytes.push_back(hi);
    uint16_t d16_val = (static_cast<uint16_t>(hi) << 8) | lo;
    std::ostringstream oss;
    oss << "JP " << formatHex16(d16_val);
    return oss.str();
}

void Instr_HALT::execute(Cpu& cpu) {
    cpu.current_instruction_cycles_ = 4;
    cpu.debug_last_instr_length_ = 1;
    cpu.debug_last_operand_ = 0;
}
std::string Instr_HALT::disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) {
    instruction_bytes.push_back(cpu.busRead(pc_at_opcode));
    return "HALT";
}

void Instr_CB_PREFIX::execute(Cpu& cpu) {
    uint8_t cb_opcode = fetch_d8_operand(cpu); 
    Instruction* cb_instr = cpu.getCbInstruction(cb_opcode);
    cb_instr->execute(cpu);
    cpu.debug_last_instr_length_ = 2; 
    cpu.debug_last_operand_ = cb_opcode; 
}
std::string Instr_CB_PREFIX::disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) {
    instruction_bytes.push_back(cpu.busRead(pc_at_opcode)); 
    uint8_t cb_sub_opcode = cpu.busRead(pc_at_opcode + 1);   
    instruction_bytes.push_back(cb_sub_opcode);
    Instruction* cb_instr_for_disasm = cpu.getCbInstruction(cb_sub_opcode);
    if (cb_instr_for_disasm) {
        std::vector<uint8_t> sub_instr_only_bytes; 
        return cb_instr_for_disasm->disassemble(cpu, pc_at_opcode + 1, sub_instr_only_bytes);
    }
    return "CB " + formatHex8(cb_sub_opcode) + " (UNKNOWN_CB)";
}

Instr_LD_MHL_R::Instr_LD_MHL_R(uint8_t src_reg) : src_reg_index_(src_reg) {}
void Instr_LD_MHL_R::execute(Cpu& cpu) {
    uint8_t value_to_store = get_reg_value(cpu, src_reg_index_); 
    cpu.busWrite(cpu.hl, value_to_store);
    cpu.current_instruction_cycles_ = 8;
    cpu.debug_last_instr_length_ = 1;
    cpu.debug_last_operand_ = 0;
}
std::string Instr_LD_MHL_R::disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) {
    instruction_bytes.push_back(cpu.busRead(pc_at_opcode));
    std::ostringstream oss;
    oss << "LD (HL), " << get_reg_name(src_reg_index_);
    return oss.str();
}

Instr_DEC_R::Instr_DEC_R(uint8_t reg) : reg_index_(reg) {}
void Instr_DEC_R::execute(Cpu& cpu) {
    uint8_t old_val;
    uint8_t new_val;
    switch (reg_index_) {
    case 0: old_val = cpu.b(); cpu.set_b(old_val - 1); new_val = cpu.b(); break;
    case 1: old_val = cpu.c(); cpu.set_c(old_val - 1); new_val = cpu.c(); break;
    case 2: old_val = cpu.d(); cpu.set_d(old_val - 1); new_val = cpu.d(); break;
    case 3: old_val = cpu.e(); cpu.set_e(old_val - 1); new_val = cpu.e(); break;
    case 4: old_val = cpu.h(); cpu.set_h(old_val - 1); new_val = cpu.h(); break;
    case 5: old_val = cpu.l(); cpu.set_l(old_val - 1); new_val = cpu.l(); break;
    case 7: old_val = cpu.a(); cpu.set_a(old_val - 1); new_val = cpu.a(); break;
    default:
        std::cerr << "Instr_DEC_R: Invalid reg_index " << (int)reg_index_ << std::endl;
        cpu.current_instruction_cycles_ = 4; cpu.debug_last_instr_length_ = 1; cpu.debug_last_operand_ = 0; return;
    }
    cpu.updateFlags_DEC8(old_val, new_val);
    cpu.current_instruction_cycles_ = 4;
    cpu.debug_last_instr_length_ = 1;
    cpu.debug_last_operand_ = 0;
}
std::string Instr_DEC_R::disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) {
    instruction_bytes.push_back(cpu.busRead(pc_at_opcode));
    std::ostringstream oss;
    oss << "DEC " << get_reg_name(reg_index_);
    return oss.str();
}

void Instr_INC_MHL::execute(Cpu& cpu) {
    uint8_t old_val = cpu.busRead(cpu.hl);
    uint8_t new_val = old_val + 1;
    cpu.busWrite(cpu.hl, new_val);
    cpu.updateFlags_INC8(old_val, new_val); 
    cpu.current_instruction_cycles_ = 12;
    cpu.debug_last_instr_length_ = 1;
    cpu.debug_last_operand_ = 0;
}
std::string Instr_INC_MHL::disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) {
    instruction_bytes.push_back(cpu.busRead(pc_at_opcode));
    return "INC (HL)";
}

void Instr_LD_MHL_D8::execute(Cpu& cpu) {
    uint8_t value = fetch_d8_operand(cpu); 
    cpu.busWrite(cpu.hl, value);
    cpu.current_instruction_cycles_ = 12;
    cpu.debug_last_instr_length_ = 2; 
    cpu.debug_last_operand_ = value;
}
std::string Instr_LD_MHL_D8::disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) {
    instruction_bytes.push_back(cpu.busRead(pc_at_opcode));
    uint8_t d8_val = cpu.busRead(pc_at_opcode + 1);
    instruction_bytes.push_back(d8_val);
    std::ostringstream oss;
    oss << "LD (HL), " << formatHex8(d8_val);
    return oss.str();
}







Instr_LD_R_R::Instr_LD_R_R(uint8_t dest_r, uint8_t src_r) : dest_reg_index_(dest_r), src_reg_index_(src_r) {}
void Instr_LD_R_R::execute(Cpu& cpu) {
    uint8_t value;
    if (src_reg_index_ == 6) { 
        value = cpu.busRead(cpu.hl);
        cpu.current_instruction_cycles_ = 8;
    } else {
        value = get_reg_value(cpu, src_reg_index_);
        cpu.current_instruction_cycles_ = 4;
    }

    if (dest_reg_index_ == 6) { 
                                
        std::cerr << "Instr_LD_R_R should not handle dest=(HL). Opcode was mis-assigned." << std::endl;
        
        
        cpu.current_instruction_cycles_ = 4; 
    } else {
        set_reg_value(cpu, dest_reg_index_, value);
    }
    

    cpu.debug_last_instr_length_ = 1;
    cpu.debug_last_operand_ = 0;
}
std::string Instr_LD_R_R::disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) {
    instruction_bytes.push_back(cpu.busRead(pc_at_opcode));
    std::ostringstream oss;
    oss << "LD " << get_reg_name(dest_reg_index_) << ", " << get_reg_name(src_reg_index_);
    return oss.str();
}


Instr_LD_A_MRR::Instr_LD_A_MRR(uint8_t rp) : rp_index_(rp) {}
void Instr_LD_A_MRR::execute(Cpu& cpu) {
    uint16_t address;
    if (rp_index_ == 0) { 
        address = cpu.bc;
    } else { 
        address = cpu.de;
    }
    cpu.set_a(cpu.busRead(address));
    cpu.current_instruction_cycles_ = 8;
    cpu.debug_last_instr_length_ = 1;
    cpu.debug_last_operand_ = 0;
}
std::string Instr_LD_A_MRR::disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) {
    instruction_bytes.push_back(cpu.busRead(pc_at_opcode));
    std::ostringstream oss;
    oss << "LD A, (" << get_rp_name(rp_index_) << ")";
    return oss.str();
}


Instr_LD_MRR_A::Instr_LD_MRR_A(uint8_t rp) : rp_index_(rp) {}
void Instr_LD_MRR_A::execute(Cpu& cpu) {
    uint16_t address;
    if (rp_index_ == 0) { 
        address = cpu.bc;
    } else { 
        address = cpu.de;
    }
    cpu.busWrite(address, cpu.a());
    cpu.current_instruction_cycles_ = 8;
    cpu.debug_last_instr_length_ = 1;
    cpu.debug_last_operand_ = 0;
}
std::string Instr_LD_MRR_A::disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) {
    instruction_bytes.push_back(cpu.busRead(pc_at_opcode));
    std::ostringstream oss;
    oss << "LD (" << get_rp_name(rp_index_) << "), A";
    return oss.str();
}


void Instr_LD_A_MA16::execute(Cpu& cpu) {
    uint16_t address = fetch_d16_operand(cpu); 
    cpu.set_a(cpu.busRead(address));
    cpu.current_instruction_cycles_ = 16;
    cpu.debug_last_instr_length_ = 3; 
    cpu.debug_last_operand_ = address; 
}
std::string Instr_LD_A_MA16::disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) {
    instruction_bytes.push_back(cpu.busRead(pc_at_opcode)); 
    uint8_t lo = cpu.busRead(pc_at_opcode + 1);
    uint8_t hi = cpu.busRead(pc_at_opcode + 2);
    instruction_bytes.push_back(lo);
    instruction_bytes.push_back(hi);
    uint16_t d16_val = (static_cast<uint16_t>(hi) << 8) | lo;
    std::ostringstream oss;
    oss << "LD A, (" << formatHex16(d16_val) << ")";
    return oss.str();
}


void Instr_LD_MA16_A::execute(Cpu& cpu) {
    uint16_t address = fetch_d16_operand(cpu); 
    cpu.busWrite(address, cpu.a());
    cpu.current_instruction_cycles_ = 16;
    cpu.debug_last_instr_length_ = 3; 
    cpu.debug_last_operand_ = address; 
}
std::string Instr_LD_MA16_A::disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) {
    instruction_bytes.push_back(cpu.busRead(pc_at_opcode)); 
    uint8_t lo = cpu.busRead(pc_at_opcode + 1);
    uint8_t hi = cpu.busRead(pc_at_opcode + 2);
    instruction_bytes.push_back(lo);
    instruction_bytes.push_back(hi);
    uint16_t d16_val = (static_cast<uint16_t>(hi) << 8) | lo;
    std::ostringstream oss;
    oss << "LD (" << formatHex16(d16_val) << "), A";
    return oss.str();
}