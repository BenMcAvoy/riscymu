#include "cpu.h"
#include "tables.h"
#include "helpers.h"

#include <array>
#include <format>
#include <print>
#include <magic_enum.hpp>

CPU::CPU()
{
    general_registers.fill(0);
    executed_instructions = 0;
    pc = 0;
}

void CPU::step_half_instruction(std::uint16_t half)
{
    throw std::runtime_error("C extension instructions are not supported yet");
}

execute_t CPU::get_instruction_handler(std::uint32_t ins) const
{
    OpGroup opgroup = static_cast<OpGroup>(extract_opcode(ins));

    switch (opgroup)
    {
    case OpGroup::Op:
        return get_op_instruction_handler(ins);
    case OpGroup::OpImm:
        return get_op_imm_instruction_handler(ins);
    case OpGroup::Load:
        return get_load_instruction_handler(ins);
    case OpGroup::Store:
        return get_store_instruction_handler(ins);
    case OpGroup::Branch:
        return get_branch_instruction_handler(ins);
    case OpGroup::Jal:
        return get_jal_instruction_handler(ins);
    case OpGroup::Jalr:
        return get_jalr_instruction_handler(ins);
    case OpGroup::Lui:
        return get_lui_instruction_handler(ins);
    case OpGroup::Auipc:
        return get_auipc_instruction_handler(ins);
    case OpGroup::EType:
        return get_etype_instruction_handler(ins);
    case OpGroup::Fence:
        return get_fence_instruction_handler(ins);
    }
}

void CPU::step_full_instruction(std::uint32_t full)
{
    (this->*get_instruction_handler(full))(full);
    pc += 4;

    executed_instructions++;
}

bool CPU::step()
{
    std::uint32_t full = mem.read<std::uint32_t>(pc);
    if ((full & 0xFFFFu) == 0) [[unlikely]]
    {
        return false;
    }

    bool compressed = (full & 0b11u) != 0b11u;

    if (compressed) [[unlikely]]
    {
        step_half_instruction(static_cast<std::uint16_t>(full));
    }
    else
    {
        step_full_instruction(full);
    }

    return true;
}

int CPU::get_mhz()
{
    auto current_time = std::chrono::high_resolution_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(current_time - start_time).count();

    double mhz = static_cast<double>(executed_instructions) / elapsed_time;
    return static_cast<int>(mhz);
}

void CPU::warm()
{
    warm_tables();

    std::println("Warmed up tables");

    start_time = std::chrono::high_resolution_clock::now();
}

void CPU::reset(bool reset_instrumentation)
{
    general_registers.fill(0);
    pc = 0;

    if (reset_instrumentation)
    {
        executed_instructions = 0;
        start_time = std::chrono::high_resolution_clock::now();
    }
}

execute_t CPU::get_op_instruction_handler(std::uint32_t ins) const
{
    auto funct3 = extract_funct3(ins);
    auto funct7 = extract_funct7(ins);

    auto idx = (funct7 << 3) | funct3;
    return op_table[idx];
}

execute_t CPU::get_op_imm_instruction_handler(std::uint32_t ins) const
{
    auto funct3 = extract_funct3(ins);
    auto imm_11_5 = extract_funct7(ins);

    auto idx = (imm_11_5 << 3) | funct3;
    return op_imm_table[idx];
}

execute_t CPU::get_load_instruction_handler(std::uint32_t ins) const
{
    auto funct3 = extract_funct3(ins);
    return load_table[funct3];
}

execute_t CPU::get_store_instruction_handler(std::uint32_t ins) const
{
    auto funct3 = extract_funct3(ins);
    return store_table[funct3];
}

execute_t CPU::get_branch_instruction_handler(std::uint32_t ins) const
{
    auto funct3 = extract_funct3(ins);
    return branch_table[funct3];
}

execute_t CPU::get_jal_instruction_handler(std::uint32_t ins) const
{
    return jal_table[0];
}

execute_t CPU::get_jalr_instruction_handler(std::uint32_t ins) const
{
    return jalr_table[0];
}

execute_t CPU::get_lui_instruction_handler(std::uint32_t ins) const
{
    return lui_table[0];
}

execute_t CPU::get_auipc_instruction_handler(std::uint32_t ins) const
{
    return auipc_table[0];
}

execute_t CPU::get_etype_instruction_handler(std::uint32_t ins) const
{
    // TODO: could be expensive here
    auto imm = extract_i_type_imm(ins);
    return etype_table[imm];
}

execute_t CPU::get_fence_instruction_handler(std::uint32_t ins) const
{
    return fence_table[0];
}

#define get(idx) get_register(idx)
#define set(idx, value) set_register(idx, value)
void CPU::execute_add(std::uint32_t ins)
{
    auto rd = static_cast<RegisterIdx>(extract_rd(ins));
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));
    auto rs2 = static_cast<RegisterIdx>(extract_rs2(ins));

    set(rd, get(rs1) + get(rs2));
}
void CPU::execute_sub(std::uint32_t ins)
{
    auto rd = static_cast<RegisterIdx>(extract_rd(ins));
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));
    auto rs2 = static_cast<RegisterIdx>(extract_rs2(ins));

    set(rd, get(rs1) - get(rs2));
}
void CPU::execute_xor(std::uint32_t ins)
{
    auto rd = static_cast<RegisterIdx>(extract_rd(ins));
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));
    auto rs2 = static_cast<RegisterIdx>(extract_rs2(ins));

    set(rd, get(rs1) ^ get(rs2));
}
void CPU::execute_or(std::uint32_t ins)
{
    auto rd = static_cast<RegisterIdx>(extract_rd(ins));
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));
    auto rs2 = static_cast<RegisterIdx>(extract_rs2(ins));

    set(rd, get(rs1) | get(rs2));
}
void CPU::execute_and(std::uint32_t ins)
{
    auto rd = static_cast<RegisterIdx>(extract_rd(ins));
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));
    auto rs2 = static_cast<RegisterIdx>(extract_rs2(ins));

    set(rd, get(rs1) & get(rs2));
}
void CPU::execute_sll(std::uint32_t ins)
{
    auto rd = static_cast<RegisterIdx>(extract_rd(ins));
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));
    auto rs2 = static_cast<RegisterIdx>(extract_rs2(ins));

    set(rd, get(rs1) << get(rs2));
}
void CPU::execute_srl(std::uint32_t ins)
{
    auto rd = static_cast<RegisterIdx>(extract_rd(ins));
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));
    auto rs2 = static_cast<RegisterIdx>(extract_rs2(ins));

    set(rd, get(rs1) >> get(rs2));
}
void CPU::execute_sra(std::uint32_t ins)
{
    auto rd = static_cast<RegisterIdx>(extract_rd(ins));
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));
    auto rs2 = static_cast<RegisterIdx>(extract_rs2(ins));

    set(rd, static_cast<std::int64_t>(get(rs1)) >> get(rs2));
}
void CPU::execute_slt(std::uint32_t ins)
{
    auto rd = static_cast<RegisterIdx>(extract_rd(ins));
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));
    auto rs2 = static_cast<RegisterIdx>(extract_rs2(ins));

    set(rd, static_cast<std::int64_t>(get(rs1)) < static_cast<std::int64_t>(get(rs2)) ? 1 : 0);
}
void CPU::execute_sltu(std::uint32_t ins)
{
    auto rd = static_cast<RegisterIdx>(extract_rd(ins));
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));
    auto rs2 = static_cast<RegisterIdx>(extract_rs2(ins));

    set(rd, get(rs1) < get(rs2) ? 1 : 0);
}
void CPU::execute_addi(std::uint32_t ins)
{
    auto rd = static_cast<RegisterIdx>(extract_rd(ins));
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));

    auto imm = sign_extend(extract_i_type_imm(ins), 12);
    set(rd, get(rs1) + imm);
}
void CPU::execute_xori(std::uint32_t ins)
{
    auto rd = static_cast<RegisterIdx>(extract_rd(ins));
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));

    auto imm = sign_extend(extract_i_type_imm(ins), 12);
    set(rd, get(rs1) ^ imm);
}
void CPU::execute_ori(std::uint32_t ins)
{
    auto rd = static_cast<RegisterIdx>(extract_rd(ins));
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));

    auto imm = sign_extend(extract_i_type_imm(ins), 12);
    set(rd, get(rs1) | imm);
}
void CPU::execute_andi(std::uint32_t ins)
{
    auto rd = static_cast<RegisterIdx>(extract_rd(ins));
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));

    auto imm = sign_extend(extract_i_type_imm(ins), 12);
    set(rd, get(rs1) & imm);
}
void CPU::execute_slli(std::uint32_t ins)
{
    auto rd = static_cast<RegisterIdx>(extract_rd(ins));
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));

    auto imm = sign_extend(extract_i_type_imm(ins), 12);

    set(rd, get(rs1) << imm);
}
void CPU::execute_srli(std::uint32_t ins)
{
    auto rd = static_cast<RegisterIdx>(extract_rd(ins));
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));

    auto imm = sign_extend(extract_i_type_imm(ins), 12);

    set(rd, get(rs1) >> imm);
}
void CPU::execute_srai(std::uint32_t ins)
{
    auto rd = static_cast<RegisterIdx>(extract_rd(ins));
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));

    auto imm = sign_extend(extract_i_type_imm(ins), 12);
    set(rd, static_cast<std::int64_t>(get(rs1)) >> imm);
}
void CPU::execute_slti(std::uint32_t ins)
{
    auto rd = static_cast<RegisterIdx>(extract_rd(ins));
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));

    auto imm = sign_extend(extract_i_type_imm(ins), 12);
    set(rd, static_cast<std::int64_t>(get(rs1)) < static_cast<std::int64_t>(imm) ? 1 : 0);
}
void CPU::execute_sltiu(std::uint32_t ins)
{
    auto rd = static_cast<RegisterIdx>(extract_rd(ins));
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));

    auto imm = sign_extend(extract_i_type_imm(ins), 12);
    set(rd, get(rs1) < static_cast<std::uint64_t>(imm) ? 1 : 0);
}
void CPU::execute_lb(std::uint32_t ins)
{
    auto rd = static_cast<RegisterIdx>(extract_rd(ins));
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));

    auto imm = sign_extend(extract_i_type_imm(ins), 12);
    set(rd, mem.read<std::int8_t>(get(rs1) + imm));
}
void CPU::execute_lh(std::uint32_t ins)
{
    auto rd = static_cast<RegisterIdx>(extract_rd(ins));
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));

    auto imm = sign_extend(extract_i_type_imm(ins), 12);
    set(rd, mem.read<std::int16_t>(get(rs1) + imm));
}
void CPU::execute_lw(std::uint32_t ins)
{
    auto rd = static_cast<RegisterIdx>(extract_rd(ins));
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));

    auto imm = sign_extend(extract_i_type_imm(ins), 12);
    set(rd, mem.read<std::int32_t>(get(rs1) + imm));
}
void CPU::execute_lbu(std::uint32_t ins)
{
    auto rd = static_cast<RegisterIdx>(extract_rd(ins));
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));

    auto imm = sign_extend(extract_i_type_imm(ins), 12);
    set(rd, mem.read<std::uint8_t>(get(rs1) + imm));
}
void CPU::execute_lhu(std::uint32_t ins)
{
    auto rd = static_cast<RegisterIdx>(extract_rd(ins));
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));

    auto imm = sign_extend(extract_i_type_imm(ins), 12);
    set(rd, mem.read<std::uint16_t>(get(rs1) + imm));
}
void CPU::execute_sb(std::uint32_t ins)
{
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));
    auto rs2 = static_cast<RegisterIdx>(extract_rs2(ins));

    auto imm = sign_extend(extract_s_type_imm(ins), 12);
    mem.write<std::uint8_t>(get(rs1) + imm, get(rs2));
}
void CPU::execute_sh(std::uint32_t ins)
{
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));
    auto rs2 = static_cast<RegisterIdx>(extract_rs2(ins));

    auto imm = sign_extend(extract_s_type_imm(ins), 12);

    mem.write<std::uint16_t>(get(rs1) + imm, get(rs2));
}
void CPU::execute_sw(std::uint32_t ins)
{
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));
    auto rs2 = static_cast<RegisterIdx>(extract_rs2(ins));

    auto imm = sign_extend(extract_s_type_imm(ins), 12);
    mem.write<std::uint32_t>(get(rs1) + imm, get(rs2));
}
void CPU::execute_beq(std::uint32_t ins)
{
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));
    auto rs2 = static_cast<RegisterIdx>(extract_rs2(ins));

    auto imm = sign_extend(extract_b_type_imm(ins), 13);

    // jump imm or 4 (default pc increment)
    pc += ((get(rs1) == get(rs2)) ? imm : 4) - 4;
}
void CPU::execute_bne(std::uint32_t ins)
{
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));
    auto rs2 = static_cast<RegisterIdx>(extract_rs2(ins));

    auto imm = sign_extend(extract_b_type_imm(ins), 13);

    // jump imm or 4 (default pc increment)
    pc += ((get(rs1) != get(rs2)) ? imm : 4) - 4;
}
void CPU::execute_blt(std::uint32_t ins)
{
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));
    auto rs2 = static_cast<RegisterIdx>(extract_rs2(ins));

    auto imm = sign_extend(extract_b_type_imm(ins), 13);

    // jump imm or 4 (default pc increment)
    pc += ((static_cast<std::int64_t>(get(rs1)) < static_cast<std::int64_t>(get(rs2))) ? imm : 4) - 4;
}
void CPU::execute_bge(std::uint32_t ins)
{
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));
    auto rs2 = static_cast<RegisterIdx>(extract_rs2(ins));

    auto imm = sign_extend(extract_b_type_imm(ins), 13);

    // jump imm or 4 (default pc increment)
    pc += ((static_cast<std::int64_t>(get(rs1)) >= static_cast<std::int64_t>(get(rs2))) ? imm : 4) - 4;
}
void CPU::execute_bltu(std::uint32_t ins)
{
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));
    auto rs2 = static_cast<RegisterIdx>(extract_rs2(ins));

    auto imm = sign_extend(extract_b_type_imm(ins), 13);

    // jump imm or 4 (default pc increment)
    pc += ((get(rs1) < get(rs2)) ? imm : 4) - 4;
}
void CPU::execute_bgeu(std::uint32_t ins)
{
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));
    auto rs2 = static_cast<RegisterIdx>(extract_rs2(ins));

    auto imm = sign_extend(extract_b_type_imm(ins), 13);

    // jump imm or 4 (default pc increment)
    pc += ((get(rs1) >= get(rs2)) ? imm : 4) - 4;
}
void CPU::execute_jal(std::uint32_t ins)
{
    auto rd = static_cast<RegisterIdx>(extract_rd(ins));

    auto imm = sign_extend(extract_j_type_imm(ins), 21);

    set(rd, pc + 4);
    pc += imm - 4;
}
void CPU::execute_jalr(std::uint32_t ins)
{
    auto rd = static_cast<RegisterIdx>(extract_rd(ins));
    auto rs1 = static_cast<RegisterIdx>(extract_rs1(ins));
    auto imm = sign_extend(extract_i_type_imm(ins), 12);

    set(rd, pc + 4);
    pc = ((get(rs1) + imm) & ~1ULL) - 4;
}
void CPU::execute_lui(std::uint32_t ins)
{
    auto rd = static_cast<RegisterIdx>(extract_rd(ins));
    auto imm = sign_extend(extract_u_type_imm(ins), 21);
    set(rd, imm);
}
void CPU::execute_auipc(std::uint32_t ins)
{
    auto rd = static_cast<RegisterIdx>(extract_rd(ins));
    auto imm = sign_extend(extract_u_type_imm(ins), 21);
    set(rd, pc + imm);
}
void CPU::execute_ecall(std::uint32_t ins) {}
void CPU::execute_ebreak(std::uint32_t ins) {}
void CPU::execute_fence(std::uint32_t ins) {}
void CPU::execute_na(std::uint32_t ins) {}
#undef get
#undef set