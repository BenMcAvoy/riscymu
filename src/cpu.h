#pragma once

#include <array>
#include <chrono>

#include "ins.h"
#include "mem.h"

struct CPUAccessor;

// Helper function to index registers
enum class RegisterIdx
{
    X0 = 0,
    X1 = 1,
    X2 = 2,
    X3 = 3,
    X4 = 4,
    X5 = 5,
    X6 = 6,
    X7 = 7,
    X8 = 8,
    X9 = 9,
    X10 = 10,
    X11 = 11,
    X12 = 12,
    X13 = 13,
    X14 = 14,
    X15 = 15,
    X16 = 16,
    X17 = 17,
    X18 = 18,
    X19 = 19,
    X20 = 20,
    X21 = 21,
    X22 = 22,
    X23 = 23,
    X24 = 24,
    X25 = 25,
    X26 = 26,
    X27 = 27,
    X28 = 28,
    X29 = 29,
    X30 = 30,
    X31 = 31,

    Zero = 0,
    Ra = 1,
    Sp = 2,
    Gp = 3,
    Tp = 4,
    T0 = 5,
    T1 = 6,
    T2 = 7,
    S0 = 8,
    FP = 8,
    S1 = 9,
    A0 = 10,
    A1 = 11,
    A2 = 12,
    A3 = 13,
    A4 = 14,
    A5 = 15,
    A6 = 16,
    A7 = 17,
    S2 = 18,
    S3 = 19,
    S4 = 20,
    S5 = 21,
    S6 = 22,
    S7 = 23,
    S8 = 24,
    S9 = 25,
    S10 = 26,
    S11 = 27,
    T3 = 28,
    T4 = 29,
    T5 = 30,
    T6 = 31,
};

class CPU;
using execute_t = void (CPU::*)(Instruction &);

class CPU
{
public:
    CPU();

    Instruction fetch_instruction();

    inline void write_mem(std::uint64_t addr, const void *data, std::size_t size)
    {
        mem.write(addr, data, size);
    }

    template <typename T>
    inline T read_mem(std::uint64_t addr) const
    {
        return mem.read<T>(addr);
    }

    inline std::uint64_t get_register(RegisterIdx idx)
    {
        return general_registers[static_cast<std::size_t>(idx)];
    }

    inline void set_register(RegisterIdx idx, std::uint64_t value)
    {
        if (idx == RegisterIdx::Zero)
        {
            return; // x0 is always zero, cannot be modified
        }

        general_registers[static_cast<std::size_t>(idx)] = value;
    }

    void set_register(std::size_t idx, std::uint64_t value)
    {
        set_register(static_cast<RegisterIdx>(idx), value);
    }
    std::uint64_t get_register(std::size_t idx)
    {
        return get_register(static_cast<RegisterIdx>(idx));
    }

    void execute_instruction(const Instruction &ins);

    int get_mhz();

    void warm();
    void reset(bool reset_instrumentation = false);

private:
    // x0-x31
    std::array<std::uint64_t, 32> general_registers;
    std::uintptr_t pc; // sp reg for the program counter

    void decode_full_r_type(Instruction &ins, std::int32_t full) const;
    void decode_full_i_type(Instruction &ins, std::int32_t full) const;
    void decode_full_s_type(Instruction &ins, std::int32_t full) const;
    void decode_full_b_type(Instruction &ins, std::int32_t full) const;
    void decode_full_u_type(Instruction &ins, std::int32_t full) const;
    void decode_full_j_type(Instruction &ins, std::int32_t full) const;

    execute_t get_instruction_handler(Instruction &ins, std::uint32_t full, OpGroup opgroup) const;

    inline execute_t decode_full_op(Instruction &ins) const;
    inline execute_t decode_full_op_imm(Instruction &ins) const;
    inline execute_t decode_full_load(Instruction &ins) const;
    inline execute_t decode_full_store(Instruction &ins) const;
    inline execute_t decode_full_branch(Instruction &ins) const;
    inline execute_t decode_full_jal(Instruction &ins) const;
    inline execute_t decode_full_jalr(Instruction &ins) const;
    inline execute_t decode_full_auipc(Instruction &ins) const;
    inline execute_t decode_full_lui(Instruction &ins) const;
    inline execute_t decode_full_etype(Instruction &ins) const;
    inline execute_t decode_full_fence(Instruction &ins) const;

    inline void fetch_full_instruction(Instruction &ins, std::uint32_t full);
    inline void fetch_half_instruction(Instruction &ins, std::uint16_t half) const;

    int executed_instructions;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;

protected:
    // Inlining won't help here
    // we use a function pointer

    void execute_add(Instruction &ins);
    void execute_sub(Instruction &ins);
    void execute_xor(Instruction &ins);
    void execute_or(Instruction &ins);
    void execute_and(Instruction &ins);
    void execute_sll(Instruction &ins);
    void execute_srl(Instruction &ins);
    void execute_sra(Instruction &ins);
    void execute_slt(Instruction &ins);
    void execute_sltu(Instruction &ins);
    void execute_addi(Instruction &ins);
    void execute_xori(Instruction &ins);
    void execute_ori(Instruction &ins);
    void execute_andi(Instruction &ins);
    void execute_slli(Instruction &ins);
    void execute_srli(Instruction &ins);
    void execute_srai(Instruction &ins);
    void execute_slti(Instruction &ins);
    void execute_sltiu(Instruction &ins);
    void execute_lb(Instruction &ins);
    void execute_lh(Instruction &ins);
    void execute_lw(Instruction &ins);
    void execute_lbu(Instruction &ins);
    void execute_lhu(Instruction &ins);
    void execute_sb(Instruction &ins);
    void execute_sh(Instruction &ins);
    void execute_sw(Instruction &ins);
    void execute_beq(Instruction &ins);
    void execute_bne(Instruction &ins);
    void execute_blt(Instruction &ins);
    void execute_bge(Instruction &ins);
    void execute_bltu(Instruction &ins);
    void execute_bgeu(Instruction &ins);
    void execute_jal(Instruction &ins);
    void execute_jalr(Instruction &ins);
    void execute_lui(Instruction &ins);
    void execute_auipc(Instruction &ins);
    void execute_ecall(Instruction &ins);
    void execute_ebreak(Instruction &ins);
    void execute_fence(Instruction &ins);
    void execute_na(Instruction &ins);

private:
    Mem<1024 * 1024> mem{}; // 1MB of memory

    friend CPUAccessor;
};