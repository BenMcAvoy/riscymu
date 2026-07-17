#pragma once

#include <array>

#include "ins.h"
#include "helpers.h"

#include "cpu.h"

struct CPUAccessor
{
    static constexpr auto make_op_table()
    {
#define MAP(funct3, funct7, handler) map[(funct7 << 3) | funct3] = handler;
        constexpr std::size_t kFunctBits = 7 + 3;
        constexpr std::size_t kTableSize = 1 << kFunctBits;

        std::array<execute_t, kTableSize> map{};
        map.fill(&CPU::execute_na);

        MAP(0x00, 0x00, &CPU::execute_add);
        MAP(0x00, 0x20, &CPU::execute_sub);

        MAP(0x04, 0x00, &CPU::execute_xor);
        MAP(0x06, 0x00, &CPU::execute_or);
        MAP(0x07, 0x00, &CPU::execute_and);

        MAP(0x01, 0x00, &CPU::execute_sll);
        MAP(0x05, 0x00, &CPU::execute_srl);
        MAP(0x05, 0x20, &CPU::execute_sra);
        MAP(0x02, 0x00, &CPU::execute_slt);
        MAP(0x03, 0x00, &CPU::execute_sltu);

        return map;
#undef MAP
    }

    static constexpr auto make_op_imm_table()
    {
// key to table should be funct3 + imm[11:5] (7 bits)
// [ imm[11:5], funct3] -> execute_t
#define MAP_DISCRIMINATED(funct3, imm_11_5, handler) map[(imm_11_5 << 3) | funct3] = handler;

// to have no discriminator we must fill every possible combination of imm[11:5] in the table to ensure they are all populated
#define MAP_UNDISCRIMINATED(funct3, handler)                    \
    for (std::uint8_t imm_11_5 = 0; imm_11_5 < 128; ++imm_11_5) \
    {                                                           \
        map[(imm_11_5 << 3) | funct3] = handler;                \
    }

        constexpr std::size_t kFunctBits = 3;
        constexpr std::size_t kImmDiscriminatorBits = 7; // (imm[11:5] for slli,srli,srai)
        constexpr std::size_t kTableSize = 1 << (kFunctBits + kImmDiscriminatorBits);

        std::array<execute_t, kTableSize> map{};
        map.fill(&CPU::execute_na);

        MAP_UNDISCRIMINATED(0x00, &CPU::execute_addi);
        MAP_UNDISCRIMINATED(0x04, &CPU::execute_xori);
        MAP_UNDISCRIMINATED(0x06, &CPU::execute_ori);
        MAP_UNDISCRIMINATED(0x07, &CPU::execute_andi);

        MAP_DISCRIMINATED(0x01, 0x00, &CPU::execute_slli);
        MAP_DISCRIMINATED(0x05, 0x00, &CPU::execute_srli);
        MAP_DISCRIMINATED(0x05, 0x20, &CPU::execute_srai);

        MAP_UNDISCRIMINATED(0x02, &CPU::execute_slti);
        MAP_UNDISCRIMINATED(0x03, &CPU::execute_sltiu);

        return map;
#undef MAP_DISCRIMINATED
#undef MAP_UNDISCRIMINATED
    }

    static constexpr auto make_load_table()
    {
#define MAP(funct3, handler) map[funct3] = handler;
        constexpr std::size_t kFunctBits = 3;
        constexpr std::size_t kTableSize = 1 << kFunctBits;
        std::array<execute_t, kTableSize> map{};
        map.fill(&CPU::execute_na);

        MAP(0x00, &CPU::execute_lb);
        MAP(0x01, &CPU::execute_lh);
        MAP(0x02, &CPU::execute_lw);
        MAP(0x04, &CPU::execute_lbu);
        MAP(0x05, &CPU::execute_lhu);

        return map;
#undef MAP
    }

    static constexpr auto make_store_table()
    {
#define MAP(funct3, handler) map[funct3] = handler;
        constexpr std::size_t kFunctBits = 3;
        constexpr std::size_t kTableSize = 1 << kFunctBits;
        std::array<execute_t, kTableSize> map{};
        map.fill(&CPU::execute_na);

        MAP(0x00, &CPU::execute_sb);
        MAP(0x01, &CPU::execute_sh);
        MAP(0x02, &CPU::execute_sw);

        return map;
#undef MAP
    }

    static constexpr auto make_branch_table()
    {
#define MAP(funct3, handler) map[funct3] = handler;
        constexpr std::size_t kFunctBits = 3;
        constexpr std::size_t kTableSize = 1 << kFunctBits;
        std::array<execute_t, kTableSize> map{};
        map.fill(&CPU::execute_na);

        MAP(0x00, &CPU::execute_beq);
        MAP(0x01, &CPU::execute_bne);
        MAP(0x04, &CPU::execute_blt);
        MAP(0x05, &CPU::execute_bge);
        MAP(0x06, &CPU::execute_bltu);
        MAP(0x07, &CPU::execute_bgeu);

        return map;
#undef MAP
    }

    static constexpr auto make_jal_table()
    {
        std::array<execute_t, 1> map{};
        map.fill(&CPU::execute_jal);
        return map;
    }

    static constexpr auto make_jalr_table()
    {
        std::array<execute_t, 1> map{};
        map.fill(&CPU::execute_jalr);
        return map;
    }

    static constexpr auto make_lui_table()
    {
        std::array<execute_t, 1> map{};
        map.fill(&CPU::execute_lui);
        return map;
    }

    static constexpr auto make_auipc_table()
    {
        std::array<execute_t, 1> map{};
        map.fill(&CPU::execute_auipc);
        return map;
    }

    static constexpr auto make_etype_table()
    {
#define MAP(imm, handler) map[imm] = handler;
        std::array<execute_t, 2> map{};
        map.fill(&CPU::execute_na);

        MAP(0, &CPU::execute_ecall);
        MAP(1, &CPU::execute_ebreak);

        return map;
#undef MAP
    }

    static constexpr auto make_fence_table()
    {
        std::array<execute_t, 1> map{};
        map.fill(&CPU::execute_fence);
        return map;
    }
};

namespace
{
    alignas(64) inline constexpr auto op_table = CPUAccessor::make_op_table();
    alignas(64) inline constexpr auto op_imm_table = CPUAccessor::make_op_imm_table();
    alignas(64) inline constexpr auto load_table = CPUAccessor::make_load_table();
    alignas(64) inline constexpr auto store_table = CPUAccessor::make_store_table();
    alignas(64) inline constexpr auto branch_table = CPUAccessor::make_branch_table();
    alignas(64) inline constexpr auto jal_table = CPUAccessor::make_jal_table();
    alignas(64) inline constexpr auto jalr_table = CPUAccessor::make_jalr_table();
    alignas(64) inline constexpr auto lui_table = CPUAccessor::make_lui_table();
    alignas(64) inline constexpr auto auipc_table = CPUAccessor::make_auipc_table();
    alignas(64) inline constexpr auto etype_table = CPUAccessor::make_etype_table();
    alignas(64) inline constexpr auto fence_table = CPUAccessor::make_fence_table();

    void warm_tables()
    {
        volatile execute_t dummy{};

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
}
