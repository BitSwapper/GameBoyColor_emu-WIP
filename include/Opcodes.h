#ifndef OPCODES_H
#define OPCODES_H

#include "Instruction.h" 


class Instr_NOP : public Instruction {
public:
    void execute(Cpu& cpu) override;
    std::string disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) override;
};


class Instr_LD_RR_D16 : public Instruction {
    uint8_t rp_index_; 
public:
    explicit Instr_LD_RR_D16(uint8_t rp);
    void execute(Cpu& cpu) override;
    std::string disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) override;
};


class Instr_INC_R : public Instruction {
    uint8_t reg_index_; 
public:
    explicit Instr_INC_R(uint8_t reg);
    void execute(Cpu& cpu) override;
    std::string disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) override;
};


class Instr_LD_R_D8 : public Instruction {
    uint8_t reg_index_; 
public:
    explicit Instr_LD_R_D8(uint8_t reg);
    void execute(Cpu& cpu) override;
    std::string disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) override;
};


class Instr_ADD_A_R : public Instruction {
    uint8_t reg_index_; 
public:
    explicit Instr_ADD_A_R(uint8_t reg);
    void execute(Cpu& cpu) override;
    std::string disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) override;
};


class Instr_SUB_A_R : public Instruction {
    uint8_t reg_index_; 
public:
    explicit Instr_SUB_A_R(uint8_t reg);
    void execute(Cpu& cpu) override;
    std::string disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) override;
};


class Instr_XOR_A : public Instruction {
public:
    void execute(Cpu& cpu) override;
    std::string disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) override;
};


class Instr_JP_A16 : public Instruction {
public:
    void execute(Cpu& cpu) override;
    std::string disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) override;
};


class Instr_HALT : public Instruction {
public:
    void execute(Cpu& cpu) override;
    std::string disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) override;
};


class Instr_CB_PREFIX : public Instruction {
public:
    void execute(Cpu& cpu) override;
    std::string disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) override;
};


class Instr_LD_MHL_R : public Instruction {
    uint8_t src_reg_index_;
public:
    explicit Instr_LD_MHL_R(uint8_t src_reg);
    void execute(Cpu& cpu) override;
    std::string disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) override;
};


class Instr_DEC_R : public Instruction {
    uint8_t reg_index_;
public:
    explicit Instr_DEC_R(uint8_t reg);
    void execute(Cpu& cpu) override;
    std::string disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) override;
};


class Instr_INC_MHL : public Instruction {
public:
    void execute(Cpu& cpu) override;
    std::string disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) override;
};


class Instr_LD_MHL_D8 : public Instruction {
public:
    void execute(Cpu& cpu) override;
    std::string disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) override;
};

class Instr_LD_R_R : public Instruction {
    uint8_t dest_reg_index_; 
    uint8_t src_reg_index_;  
public:
    explicit Instr_LD_R_R(uint8_t dest_r, uint8_t src_r);
    void execute(Cpu& cpu) override;
    std::string disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) override;
};

class Instr_LD_A_MRR : public Instruction {
    uint8_t rp_index_; 
public:
    explicit Instr_LD_A_MRR(uint8_t rp);
    void execute(Cpu& cpu) override;
    std::string disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) override;
};


class Instr_LD_MRR_A : public Instruction {
    uint8_t rp_index_; 
public:
    explicit Instr_LD_MRR_A(uint8_t rp);
    void execute(Cpu& cpu) override;
    std::string disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) override;
};


class Instr_LD_A_MA16 : public Instruction {
public:
    void execute(Cpu& cpu) override;
    std::string disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) override;
};


class Instr_LD_MA16_A : public Instruction {
public:
    void execute(Cpu& cpu) override;
    std::string disassemble(Cpu& cpu, uint16_t pc_at_opcode, std::vector<uint8_t>& instruction_bytes) override;
};

#endif 