#include "cpu.h"
#include "tables.h"
#include "helpers.h"

#include <array>
#include <format>
#include <print>
#include <magic_enum.hpp>

constexpr std::size_t kRegisterCount = 32;

CPU::CPU()
{
    general_registers.fill(0);
    pc = 0;
    executed_instructions = 0;
}

void CPU::fetch_half_instruction(Instruction &ins, std::int16_t half) const
{
    throw std::runtime_error("C extension instructions are not supported yet");
}

void CPU::decode_full_r_type(Instruction &ins, std::int32_t full) const
{
    ins.format = Format::R;
    ins.rd = extract_rd(full);
    ins.rs1 = extract_rs1(full);
    ins.rs2 = extract_rs2(full);
}
void CPU::decode_full_i_type(Instruction &ins, std::int32_t full) const
{
    ins.format = Format::I;
    ins.rd = extract_rd(full);
    ins.rs1 = extract_rs1(full);
    ins.imm = sign_extend(extract(full, 20, 31), 12);
}
void CPU::decode_full_s_type(Instruction &ins, std::int32_t full) const
{
    ins.format = Format::S;
    ins.rs1 = extract_rs1(full);
    ins.rs2 = extract_rs2(full);

    auto imm4_0 = extract(full, 7, 11);   // bits 11:7
    auto imm11_5 = extract(full, 25, 31); // bits 31:25
    auto imm = (imm11_5 << 5) | imm4_0;   // combine to get 12-bit immediate

    ins.imm = sign_extend(imm, 12);
}
void CPU::decode_full_b_type(Instruction &ins, std::int32_t full) const
{
    ins.format = Format::B;

    ins.rs1 = extract_rs1(full);
    ins.rs2 = extract_rs2(full);

    auto imm_1_4 = extract(full, 8, 11);
    auto imm_5_10 = extract(full, 25, 30);
    auto imm_11 = extract(full, 7);
    auto imm_12 = extract(full, 31);
    auto imm = (imm_12 << 12) | (imm_11 << 11) | (imm_5_10 << 5) | (imm_1_4 << 1);

    ins.imm = sign_extend(imm, 13);
}
void CPU::decode_full_u_type(Instruction &ins, std::int32_t full) const
{
    ins.format = Format::U;
    ins.rd = extract_rd(full);

    auto imm = extract(full, 12, 31) << 12;
    ins.imm = sign_extend(imm, 32);
}
void CPU::decode_full_j_type(Instruction &ins, std::int32_t full) const
{
    ins.format = Format::J;
    ins.rd = extract_rd(full);

    auto imm1_10 = extract(full, 21, 30);  // bits 30:21
    auto imm11 = extract(full, 20);        // bit 20
    auto imm12_19 = extract(full, 12, 19); // bits 19:12
    auto imm20 = extract(full, 31);        // bit 31
    auto imm = (imm20 << 20) | (imm12_19 << 12) | (imm11 << 11) | (imm1_10 << 1);

    ins.imm = sign_extend(imm, 21);
}

void CPU::decode_full_opcode(Instruction &ins, std::int32_t full, OpGroup opgroup) const
{
    switch (opgroup)
    {
    case OpGroup::Op:
        ins.op = decode_full_op(ins.raw);
        break;
    case OpGroup::OpImm:
        ins.op = decode_full_op_imm(ins.raw);
        break;
    case OpGroup::Load:
        ins.op = decode_full_load(ins.raw);
        break;
    case OpGroup::Store:
        ins.op = decode_full_store(ins.raw);
        break;
    case OpGroup::Branch:
        ins.op = decode_full_branch(ins.raw);
        break;
    case OpGroup::Jal:
        ins.op = decode_full_jal(ins.raw);
        break;
    case OpGroup::Jalr:
        ins.op = decode_full_jalr(ins.raw);
        break;
    case OpGroup::Lui:
        ins.op = decode_full_lui(ins.raw);
        break;
    case OpGroup::Auipc:
        ins.op = decode_full_auipc(ins.raw);
        break;
    case OpGroup::EType:
        ins.op = decode_full_etype(ins.raw);
        break;
    case OpGroup::Fence:
        ins.op = decode_full_fence(ins.raw);
        break;
    }
}

void CPU::fetch_full_instruction(Instruction &ins, std::int32_t full) const
{
    OpGroup opgroup = static_cast<OpGroup>(extract_opcode(full));
    ins.opgroup = opgroup;

    // std::println("Fetched instruction: 0x{:08x}, opgroup: 0x{:02x}", full, static_cast<std::underlying_type_t<OpGroup>>(opgroup));

    switch (opgroup)
    {
    case OpGroup::Op:
        decode_full_r_type(ins, full);
        break;

    case OpGroup::OpImm:
        decode_full_i_type(ins, full);
        break;

    case OpGroup::Load:
        decode_full_i_type(ins, full);
        break;

    case OpGroup::Store:
        decode_full_s_type(ins, full);
        break;

    case OpGroup::Branch:
        decode_full_b_type(ins, full);
        break;

    case OpGroup::Jal:
        decode_full_j_type(ins, full);
        break;

    case OpGroup::Jalr:
        decode_full_i_type(ins, full);
        break;

    case OpGroup::Lui:
        decode_full_u_type(ins, full);
        break;

    case OpGroup::Auipc:
        decode_full_u_type(ins, full);
        break;

    case OpGroup::EType:
        decode_full_i_type(ins, full);
        break;

    case OpGroup::Fence:
        decode_full_i_type(ins, full);
        break;

    default:
        throw std::runtime_error("Unsupported instruction format");
    }

    decode_full_opcode(ins, full, opgroup);
}

Instruction CPU::fetch_instruction() const
{
    if (start_time.time_since_epoch().count() == 0)
    {
        start_time = std::chrono::high_resolution_clock::now();
    }

    Instruction ins;

    std::uint16_t half = mem.read<std::uint16_t>(pc);
    if (half == 0)
    {
        return ins;
    }

    bool compressed = (half & bit_mask(2)) != bit_mask(2);

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

void CPU::execute_instruction(const Instruction &ins)
{
#define get(idx) get_register(static_cast<RegisterIdx>(idx))
#define set(idx, value) set_register(static_cast<RegisterIdx>(idx), value)

    auto &reg = general_registers;

    bool is_branch = (ins.opgroup == OpGroup::Branch) || (ins.opgroup == OpGroup::Jal) || (ins.opgroup == OpGroup::Jalr);
    bool increment_pc = !is_branch;

    switch (ins.op)
    {
    case OpCode::Add:
        set(ins.rd, get(ins.rs1) + get(ins.rs2));
        break;
    case OpCode::Sub:
        set(ins.rd, get(ins.rs1) - get(ins.rs2));
        break;
    case OpCode::Xor:
        set(ins.rd, get(ins.rs1) ^ get(ins.rs2));
        break;
    case OpCode::Or:
        set(ins.rd, get(ins.rs1) | get(ins.rs2));
        break;
    case OpCode::And:
        set(ins.rd, get(ins.rs1) & get(ins.rs2));
        break;
    case OpCode::Sll:
        set(ins.rd, get(ins.rs1) << get(ins.rs2));
        break;
    case OpCode::Srl:
        set(ins.rd, get(ins.rs1) >> get(ins.rs2));
        break;
    case OpCode::Sra:
        set(ins.rd, static_cast<std::int64_t>(get(ins.rs1)) >> get(ins.rs2));
        break;
    case OpCode::Slt:
        set(ins.rd, static_cast<std::int64_t>(get(ins.rs1)) < static_cast<std::int64_t>(get(ins.rs2)) ? 1 : 0);
        break;
    case OpCode::Sltu:
        set(ins.rd, get(ins.rs1) < get(ins.rs2) ? 1 : 0);
        break;
    case OpCode::Addi:
        set(ins.rd, get(ins.rs1) + ins.imm);
        break;
    case OpCode::Xori:
        set(ins.rd, get(ins.rs1) ^ ins.imm);
        break;
    case OpCode::Ori:
        set(ins.rd, get(ins.rs1) | ins.imm);
        break;
    case OpCode::Andi:
        set(ins.rd, get(ins.rs1) & ins.imm);
        break;
    case OpCode::Slli:
        set(ins.rd, get(ins.rs1) << ins.imm);
        break;
    case OpCode::Srli:
        set(ins.rd, get(ins.rs1) >> ins.imm);
        break;
    case OpCode::Srai:
        set(ins.rd, static_cast<std::int64_t>(get(ins.rs1)) >> ins.imm);
        break;
    case OpCode::Slti:
        set(ins.rd, static_cast<std::int64_t>(get(ins.rs1)) < static_cast<std::int64_t>(ins.imm) ? 1 : 0);
        break;
    case OpCode::Sltiu:
        set(ins.rd, get(ins.rs1) < static_cast<uint64_t>(ins.imm) ? 1 : 0);
        break;
    case OpCode::Lb:
        set(ins.rd, mem.read<std::int8_t>(get(ins.rs1) + ins.imm));
        break;
    case OpCode::Lh:
        set(ins.rd, mem.read<std::int16_t>(get(ins.rs1) + ins.imm));
        break;
    case OpCode::Lw:
        set(ins.rd, mem.read<std::int32_t>(get(ins.rs1) + ins.imm));
        break;
    case OpCode::Lbu:
        set(ins.rd, mem.read<std::uint8_t>(get(ins.rs1) + ins.imm));
        break;
    case OpCode::Lhu:
        set(ins.rd, mem.read<std::uint16_t>(get(ins.rs1) + ins.imm));
        break;
    case OpCode::Sb:
        mem.write<std::uint8_t>(get(ins.rs1) + ins.imm, get(ins.rs2));
        break;
    case OpCode::Sh:
        mem.write<std::uint16_t>(get(ins.rs1) + ins.imm, get(ins.rs2));
        break;
    case OpCode::Sw:
        mem.write<std::uint32_t>(get(ins.rs1) + ins.imm, get(ins.rs2));
        break;
    case OpCode::Beq:
        pc += (get(ins.rs1) == get(ins.rs2)) ? ins.imm : ins.length;
        break;
    case OpCode::Bne:
        pc += (get(ins.rs1) != get(ins.rs2)) ? ins.imm : ins.length;
        break;
    case OpCode::Blt:
        pc += (static_cast<std::int64_t>(get(ins.rs1)) < static_cast<std::int64_t>(get(ins.rs2))) ? ins.imm : ins.length;
        break;
    case OpCode::Bge:
        pc += (static_cast<std::int64_t>(get(ins.rs1)) >= static_cast<std::int64_t>(get(ins.rs2))) ? ins.imm : ins.length;
        break;
    case OpCode::Bltu:
        pc += (get(ins.rs1) < get(ins.rs2)) ? ins.imm : ins.length;
        break;
    case OpCode::Bgeu:
        pc += (get(ins.rs1) >= get(ins.rs2)) ? ins.imm : ins.length;
        break;
    case OpCode::Jal:
        set(ins.rd, pc + 4);
        pc += ins.imm;
        break;
    case OpCode::Jalr:
        set(ins.rd, pc + 4);
        pc = (get(ins.rs1) + ins.imm) & ~1ULL;
        break;
    case OpCode::Lui:
        set(ins.rd, ins.imm);
        break;
    case OpCode::Auipc:
        set(ins.rd, pc + ins.imm);
        break;
    case OpCode::Ecall:
        // TODO: handle
        throw std::runtime_error("Ecall instruction not implemented yet");
        break;
    case OpCode::Ebreak:
        // TODO: handle
        throw std::runtime_error("Ebreak instruction not implemented yet");
        break;
    case OpCode::Fence:
        // TODO: handle
        throw std::runtime_error("Fence instruction not implemented yet");
        break;
    case OpCode::NA:
        throw std::runtime_error("Attempted to execute an instruction with OpCode::NA");
        break;
    default:
        throw std::runtime_error("Attempted to execute an instruction with an unknown OpCode");
        break;
    }

    if (increment_pc)
    {
        pc += ins.length;
    }

    executed_instructions++;

#undef get
#undef set
}

int CPU::get_mhz()
{
    auto current_time = std::chrono::high_resolution_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(current_time - start_time).count();

    if (elapsed_time == 0)
    {
        return 0;
    }

    double mhz = static_cast<double>(executed_instructions) / elapsed_time;
    return static_cast<int>(mhz);
}

void CPU::warm()
{
    warm_tables();
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