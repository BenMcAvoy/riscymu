#include "cpu.h"
#include "tables.h"

#include <format>
#include <print>

CPU::CPU()
{
    general_registers.fill(0);
    pc = 0;
}

void CPU::fetch_half_instruction(Instruction &ins, std::int16_t half) const
{
    throw std::runtime_error("fetch_half_instruction not implemented");
}

void CPU::fetch_full_instruction_r_type(Instruction &ins, std::int32_t full) const
{
    auto funct3 = (full >> 12) & 0b111;
    auto funct7 = (full >> 25) & 0b1111111;

    // key to differentiate between instructions with the same opcode
    std::uint16_t key = (funct7 << 3) | funct3;

    constexpr auto r_table = make_r_table();
    OpCode op = r_table[key];

    auto idx = static_cast<std::underlying_type_t<OpCode>>(op);
    if (idx >= static_cast<std::underlying_type_t<OpCode>>(OpCode::Add) &&
        idx <= static_cast<std::underlying_type_t<OpCode>>(OpCode::Sltu))
    {
        ins.op = op;
    }
    else
    {
        throw std::runtime_error("Unsupported R-type instruction");
    }

    // TODO: Other stuff
}

void CPU::fetch_full_instruction_i_type(Instruction &ins, std::int32_t full) const
{
    auto funct3 = (full >> 12) & 0b111;

    constexpr auto i_table = make_i_table();
    OpCode op = i_table[funct3];

    auto idx = static_cast<std::underlying_type_t<OpCode>>(op);
    if (idx >= static_cast<std::underlying_type_t<OpCode>>(OpCode::Addi) &&
        idx <= static_cast<std::underlying_type_t<OpCode>>(OpCode::Sltiu))
    {
        ins.op = op;
    }
    else
    {
        throw std::runtime_error("Unsupported I-type instruction");
    }

    // TODO: Other stuff
}

void CPU::fetch_full_instruction(Instruction &ins, std::int32_t full) const
{
    // 7 bits for opcode
    OpGroup op_group = static_cast<OpGroup>(full & 0b1111111);
    ins.op_group = op_group;

    std::println("Fetched instruction: 0x{:08x}, op_group: 0x{:02x}", full, static_cast<std::underlying_type_t<OpGroup>>(op_group));

    switch (op_group)
    {
    case OpGroup::Op:
    {
        // R-type instruction
        ins.format = Format::R;
        ins.rd = (full >> 7) & 0b11111;
        ins.rs1 = (full >> 15) & 0b11111;
        ins.rs2 = (full >> 20) & 0b11111;
        fetch_full_instruction_r_type(ins, full);

        break;
    }

    case OpGroup::OpImm:
    {
        // I-type instruction
        ins.format = Format::I;
        ins.rd = (full >> 7) & 0b11111;
        ins.rs1 = (full >> 15) & 0b11111;
        ins.imm = (full >> 20) & 0b111111111111; // 12 bits
        fetch_full_instruction_i_type(ins, full);

        break;
    }

    default:
        throw std::runtime_error("Unsupported instruction format");
    }
}

Instruction CPU::fetch_instruction() const
{
    Instruction ins;

    std::uint16_t half = mem.read<std::uint16_t>(pc);
    bool compressed = (half & 0b11) != 0b11;

    ins.raw = half;
    ins.length = compressed ? 2 : 4;

    if (!compressed)
    {
        std::uint16_t half2 = mem.read<std::uint16_t>(pc + 2);
        ins.raw |= (half2 << 16);
    }

    if (compressed)
    {
        fetch_half_instruction(ins, half);
    }
    else
    {
        std::uint16_t half2 = mem.read<std::uint16_t>(pc + 2);
        fetch_full_instruction(ins, half | (half2 << 16));
    }

    return ins;
}