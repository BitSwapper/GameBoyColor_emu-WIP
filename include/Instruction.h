#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <string>
#include <vector>
#include <cstdint>
#include <memory> 

class Cpu;

class Instruction {
public:
    virtual ~Instruction() = default;

    virtual void execute(Cpu& cpu) = 0;

    virtual std::string disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) = 0;

    virtual bool isInvalid() const {
        return false;
    }
};

#endif 