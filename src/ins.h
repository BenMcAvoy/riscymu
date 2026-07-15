#pragma once

#include <cstdint>
#include <string>

enum class Format : std::uint8_t
{
    R,
    I,
    S,
    B,
    U,
    J,
    NA,
};

enum class OpGroup : std::uint8_t
{
    Op = 0b0110011,
    OpImm = 0b0010011,
    Load = 0b0000011,
    Store = 0b0100011,
    Branch = 0b1100011,
    Jal = 0b1101111,
    Jalr = 0b1100111,
    Lui = 0b0110111,
    Auipc = 0b0010111,
    EType = 0b1110011,
    Fence = 0b0001111,
    NA,
};

// NOTE: this is not the direct opcode from the instruction
// this is a decoded opcode that is used to differentiate between instructions
// with the same binary opcode, e.g. addi and addiw have the same binary opcode,
// but different decoded opcodes
enum class OpCode : std::uint8_t
{
    Add,
    Sub,
    Xor,
    Or,
    And,
    Sll,
    Srl,
    Sra,
    Slt,
    Sltu,
    Addi,
    Xori,
    Ori,
    Andi,
    Slli,
    Srli,
    Srai,
    Slti,
    Sltiu,
    Lb,
    Lh,
    Lw,
    Lbu,
    Lhu,
    Sb,
    Sh,
    Sw,
    Beq,
    Bne,
    Blt,
    Bge,
    Bltu,
    Bgeu,
    Jal,
    Jalr,
    Lui,
    Auipc,
    Ecall,
    Ebreak,
    Fence,
    NA,
};

struct Instruction
{
    Instruction() = default;

    // helper for debug logging
    std::string to_string() const;

    // original
    std::uint32_t raw = 0;
    Format format = Format::NA;
    OpGroup opgroup = OpGroup::NA;

    // actual opcode
    OpCode op = OpCode::NA;

    // destination register, used for both source and destination in some instructions
    // e.g. addi x1, x2, 5 -> rd = x1
    std::uint8_t rd = 0;
    // source register 1
    std::uint8_t rs1 = 0;
    // source register 2
    std::uint8_t rs2 = 0;

    // immediate value, used for stuff like addi, anything with an immediate value, or for branch offsets
    // important! shamt may be crammed into this field in the upper bits, so make sure to mask it before using it
    // it contains the signed *and* unsigned values, one being the full signed value and one being the unsigned value for things like shamt
    std::int64_t imm = 0;

    // because of c extension, it could be 2 bytes instead of 4
    std::uint8_t length = 0;
};