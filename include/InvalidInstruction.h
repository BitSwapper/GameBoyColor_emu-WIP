#ifndef INVALID_INSTRUCTION_H
#define INVALID_INSTRUCTION_H

#include "Instruction.h"

class InvalidInstruction : public Instruction {
public:
    explicit InvalidInstruction(uint8_t opcode_val);
    void execute(Cpu& cpu) override;
    std::string disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) override;

    bool isInvalid() const override {
        return true;
    }

private:
    uint8_t illegal_opcode_value_;
};

#endif 