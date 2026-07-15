#pragma once

#include <array>
#include "ins.h"
#include "helpers.h"

namespace
{
    constexpr auto make_op_table()
    {
#define MAP(funct3, funct7, opcode) map[(funct7 << 3) | funct3] = OpCode::opcode;
        constexpr std::size_t kFunctBits = 7 + 3;
        constexpr std::size_t kTableSize = 1 << kFunctBits;

        std::array<OpCode, kTableSize> map{};
        map.fill(OpCode::NA);

        MAP(0x00, 0x00, Add);
        MAP(0x00, 0x20, Sub);

        MAP(0x04, 0x00, Xor);
        MAP(0x06, 0x00, Or);
        MAP(0x07, 0x00, And);

        MAP(0x01, 0x00, Sll);
        MAP(0x05, 0x00, Srl);
        MAP(0x05, 0x20, Sra);
        MAP(0x02, 0x00, Slt);
        MAP(0x03, 0x00, Sltu);

        return map;
#undef MAP
    }

    constexpr auto make_op_imm_table()
    {
// key to table should be funct3 + imm[11:5] (7 bits)
// [ imm[11:5], funct3] -> OpCode
#define MAP_DISCRIMINATED(funct3, imm_11_5, opcode) map[(imm_11_5 << 3) | funct3] = OpCode::opcode;

// to have no discriminator we must fill every possible combination of imm[11:5] in the table to ensure they are all populated
#define MAP_UNDISCRIMINATED(funct3, opcode)                     \
    for (std::uint8_t imm_11_5 = 0; imm_11_5 < 128; ++imm_11_5) \
    {                                                           \
        map[(imm_11_5 << 3) | funct3] = OpCode::opcode;         \
    }

        constexpr std::size_t kFunctBits = 3;
        constexpr std::size_t kImmDiscriminatorBits = 7; // (imm[11:5] for slli,srli,srai)
        constexpr std::size_t kTableSize = 1 << (kFunctBits + kImmDiscriminatorBits);

        std::array<OpCode, kTableSize> map{};
        map.fill(OpCode::NA);

        MAP_UNDISCRIMINATED(0x00, Addi);
        MAP_UNDISCRIMINATED(0x04, Xori);
        MAP_UNDISCRIMINATED(0x06, Ori);
        MAP_UNDISCRIMINATED(0x07, Andi);

        MAP_DISCRIMINATED(0x01, 0x00, Slli);
        MAP_DISCRIMINATED(0x05, 0x00, Srli);
        MAP_DISCRIMINATED(0x05, 0x20, Srai);

        MAP_UNDISCRIMINATED(0x02, Slti);
        MAP_UNDISCRIMINATED(0x03, Sltiu);

        return map;
#undef MAP_DISCRIMINATED
#undef MAP_UNDIMINISCRATED
    }

    constexpr auto make_load_table()
    {
#define MAP(funct3, opcode) map[funct3] = OpCode::opcode;
        constexpr std::size_t kFunctBits = 3;
        constexpr std::size_t kTableSize = 1 << kFunctBits;
        std::array<OpCode, kTableSize> map{};
        map.fill(OpCode::NA);

        MAP(0x00, Lb);
        MAP(0x01, Lh);
        MAP(0x02, Lw);
        MAP(0x04, Lbu);
        MAP(0x05, Lhu);

        return map;
#undef MAP
    }

    constexpr auto make_store_table()
    {
#define MAP(funct3, opcode) map[funct3] = OpCode::opcode;
        constexpr std::size_t kFunctBits = 3;
        constexpr std::size_t kTableSize = 1 << kFunctBits;
        std::array<OpCode, kTableSize> map{};
        map.fill(OpCode::NA);

        MAP(0x00, Sb);
        MAP(0x01, Sh);
        MAP(0x02, Sw);

        return map;
#undef MAP
    }

    constexpr auto make_branch_table()
    {
#define MAP(funct3, opcode) map[funct3] = OpCode::opcode;
        constexpr std::size_t kFunctBits = 3;
        constexpr std::size_t kTableSize = 1 << kFunctBits;
        std::array<OpCode, kTableSize> map{};
        map.fill(OpCode::NA);

        MAP(0x00, Beq);
        MAP(0x01, Bne);
        MAP(0x04, Blt);
        MAP(0x05, Bge);
        MAP(0x06, Bltu);
        MAP(0x07, Bgeu);

        return map;
#undef MAP
    }

    constexpr auto make_jal_table()
    {
        std::array<OpCode, 1> map{};
        map.fill(OpCode::Jal);
        return map;
    }

    constexpr auto make_jalr_table()
    {
        std::array<OpCode, 1> map{};
        map.fill(OpCode::Jalr);
        return map;
    }

    constexpr auto make_lui_table()
    {
        std::array<OpCode, 1> map{};
        map.fill(OpCode::Lui);
        return map;
    }

    constexpr auto make_auipc_table()
    {
        std::array<OpCode, 1> map{};
        map.fill(OpCode::Auipc);
        return map;
    }

    constexpr auto make_etype_table()
    {
#define MAP(imm, opcode) map[imm] = OpCode::opcode;

        std::array<OpCode, 2> map{};
        MAP(0, Ecall);
        MAP(1, Ebreak);
        return map;

#undef MAP
    }

    constexpr auto make_fence_table()
    {
        std::array<OpCode, 1> map{};
        map.fill(OpCode::Fence);
        return map;
    }
}

alignas(64) static inline constexpr auto op_table = make_op_table();
alignas(64) static inline constexpr auto op_imm_table = make_op_imm_table();
alignas(64) static inline constexpr auto load_table = make_load_table();
alignas(64) static inline constexpr auto store_table = make_store_table();
alignas(64) static inline constexpr auto branch_table = make_branch_table();
alignas(64) static inline constexpr auto jal_table = make_jal_table();
alignas(64) static inline constexpr auto jalr_table = make_jalr_table();
alignas(64) static inline constexpr auto lui_table = make_lui_table();
alignas(64) static inline constexpr auto auipc_table = make_auipc_table();
alignas(64) static inline constexpr auto etype_table = make_etype_table();
alignas(64) static inline constexpr auto fence_table = make_fence_table();

void warm_tables()
{
    volatile OpCode dummy{};

    for (auto v : op_table)
        dummy = v;
    for (auto v : op_imm_table)
        dummy = v;
    for (auto v : load_table)
        dummy = v;
    for (auto v : store_table)
        dummy = v;
    for (auto v : branch_table)
        dummy = v;
    for (auto v : jal_table)
        dummy = v;
    for (auto v : jalr_table)
        dummy = v;
    for (auto v : lui_table)
        dummy = v;
    for (auto v : auipc_table)
        dummy = v;
    for (auto v : etype_table)
        dummy = v;
    for (auto v : fence_table)
        dummy = v;

    (void)dummy;
}