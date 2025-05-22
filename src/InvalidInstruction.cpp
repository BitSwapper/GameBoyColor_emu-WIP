#include "InvalidInstruction.h"
#include "Cpu.h"   
#include "Utils.h" 
#include <iostream>
#include <sstream>

InvalidInstruction::InvalidInstruction(uint8_t opcode_val) : illegal_opcode_value_(opcode_val) {}

void InvalidInstruction::execute(Cpu& cpu) {
    std::cerr << "Error: Executing Invalid/Unimplemented Opcode: " << formatHex8(illegal_opcode_value_)
        << " at PC: " << formatHex16(cpu.debug_last_instr_pc_) << std::endl;
    cpu.current_instruction_cycles_ = 4;
    cpu.debug_last_operand_ = 0;
    cpu.debug_last_instr_length_ = 1;
}

std::string InvalidInstruction::disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) {

    instruction_bytes.push_back(illegal_opcode_value_);

    std::ostringstream oss;
    oss << "DB " << formatHex8(illegal_opcode_value_) << " (INVALID)";
    return oss.str();
}