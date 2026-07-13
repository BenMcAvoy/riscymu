#pragma once

#include <array>
#include "ins.h"

constexpr auto make_r_table()
{
#define IDX(funct7, funct3) ((funct7 << 3) | funct3)
#define MAP(funct7, funct3, opcode) r_table[IDX(funct7, funct3)] = OpCode::opcode;

    // funct7 | funct3 = 10 bits, 1024 entries.
    constexpr std::size_t kFunctBits = 7 + 3;
    constexpr std::size_t kTableSize = 1 << kFunctBits;

    std::array<OpCode, kTableSize> r_table{};

    MAP(0b0000000, 0b000, Add);
    MAP(0b0100000, 0b000, Sub);

    MAP(0b0000000, 0b100, Xor);
    MAP(0b0000000, 0b110, Or);
    MAP(0b0000000, 0b111, And);

    MAP(0b0000000, 0b001, Sll);
    MAP(0b0000000, 0b101, Srl);
    MAP(0b0100000, 0b101, Sra);

    MAP(0b0000000, 0b010, Slt);
    MAP(0b0000000, 0b011, Sltu);

    return r_table;

#undef IDX
#undef MAP
}

constexpr auto make_i_table()
{
#define MAP(funct3, opcode) i_table[funct3] = OpCode::opcode;

    std::array<OpCode, 8> i_table{};

    MAP(0b000, Addi);
    MAP(0b100, Xori);
    MAP(0b110, Ori);
    MAP(0b111, Andi);

    MAP(0b001, Slli);
    MAP(0b101, Srli); // funct7 == 0b0000000
    MAP(0b101, Srai); // funct7 == 0b0100000

    MAP(0b010, Slti);
    MAP(0b011, Sltiu);

    return i_table;

#undef MAP
}

constexpr auto make_load_table()
{
#define MAP(funct3, opcode) load_table[funct3] = OpCode::opcode;

    std::array<OpCode, 8> load_table{};

    MAP(0b000, Lb);
    MAP(0b001, Lh);
    MAP(0b010, Lw);
    MAP(0b100, Lbu);
    MAP(0b101, Lhu);

    return load_table;

#undef MAP
}

constexpr auto make_store_table()
{
#define MAP(funct3, opcode) store_table[funct3] = OpCode::opcode;

    std::array<OpCode, 8> store_table{};

    MAP(0b000, Sb);
    MAP(0b001, Sh);
    MAP(0b010, Sw);

    return store_table;

#undef MAP
}

constexpr auto make_branch_table()
{
#define MAP(funct3, opcode) branch_table[funct3] = OpCode::opcode;

    std::array<OpCode, 8> branch_table{};

    MAP(0b000, Beq);
    MAP(0b001, Bne);
    MAP(0b100, Blt);
    MAP(0b101, Bge);
    MAP(0b110, Bltu);
    MAP(0b111, Bgeu);

    return branch_table;

#undef MAP
}

// all other tables are small enough that it is not worth it