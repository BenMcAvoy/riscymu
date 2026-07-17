#pragma once

#include <array>
#include <chrono>

#include "ins.h"
#include "mem.h"

struct CPUAccessor;

// Helper function to index registers
enum class RegisterIdx : std::uint8_t
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
using execute_t = void (CPU::*)(std::uint32_t);

class CPU
{
public:
    CPU();

    bool step();

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

    // Instrumentation API
    void warm();
    int get_mhz();
    void reset(bool reset_instrumentation = false);

private:
    // x0-x31
    std::array<std::uint64_t, 32> general_registers;
    std::uintptr_t pc; // sp reg for the program counter

    /*
    void decode_full_r_type(Instruction &ins, std::int32_t full) const;
    void decode_full_i_type(Instruction &ins, std::int32_t full) const;
    void decode_full_s_type(Instruction &ins, std::int32_t full) const;
    void decode_full_b_type(Instruction &ins, std::int32_t full) const;
    void decode_full_u_type(Instruction &ins, std::int32_t full) const;
    void decode_full_j_type(Instruction &ins, std::int32_t full) const;
    */

    inline execute_t get_instruction_handler(std::uint32_t ins) const;

    inline execute_t get_op_instruction_handler(std::uint32_t ins) const;
    inline execute_t get_op_imm_instruction_handler(std::uint32_t ins) const;
    inline execute_t get_load_instruction_handler(std::uint32_t ins) const;
    inline execute_t get_store_instruction_handler(std::uint32_t ins) const;
    inline execute_t get_branch_instruction_handler(std::uint32_t ins) const;
    inline execute_t get_jal_instruction_handler(std::uint32_t ins) const;
    inline execute_t get_jalr_instruction_handler(std::uint32_t ins) const;
    inline execute_t get_auipc_instruction_handler(std::uint32_t ins) const;
    inline execute_t get_lui_instruction_handler(std::uint32_t ins) const;
    inline execute_t get_etype_instruction_handler(std::uint32_t ins) const;
    inline execute_t get_fence_instruction_handler(std::uint32_t ins) const;

    inline void step_full_instruction(std::uint32_t full);
    inline void step_half_instruction(std::uint16_t half);

    int executed_instructions;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;

protected:
    // Inlining won't help here
    // we use a function pointer

    void execute_add(std::uint32_t ins);
    void execute_sub(std::uint32_t ins);
    void execute_xor(std::uint32_t ins);
    void execute_or(std::uint32_t ins);
    void execute_and(std::uint32_t ins);
    void execute_sll(std::uint32_t ins);
    void execute_srl(std::uint32_t ins);
    void execute_sra(std::uint32_t ins);
    void execute_slt(std::uint32_t ins);
    void execute_sltu(std::uint32_t ins);
    void execute_addi(std::uint32_t ins);
    void execute_xori(std::uint32_t ins);
    void execute_ori(std::uint32_t ins);
    void execute_andi(std::uint32_t ins);
    void execute_slli(std::uint32_t ins);
    void execute_srli(std::uint32_t ins);
    void execute_srai(std::uint32_t ins);
    void execute_slti(std::uint32_t ins);
    void execute_sltiu(std::uint32_t ins);
    void execute_lb(std::uint32_t ins);
    void execute_lh(std::uint32_t ins);
    void execute_lw(std::uint32_t ins);
    void execute_lbu(std::uint32_t ins);
    void execute_lhu(std::uint32_t ins);
    void execute_sb(std::uint32_t ins);
    void execute_sh(std::uint32_t ins);
    void execute_sw(std::uint32_t ins);
    void execute_beq(std::uint32_t ins);
    void execute_bne(std::uint32_t ins);
    void execute_blt(std::uint32_t ins);
    void execute_bge(std::uint32_t ins);
    void execute_bltu(std::uint32_t ins);
    void execute_bgeu(std::uint32_t ins);
    void execute_jal(std::uint32_t ins);
    void execute_jalr(std::uint32_t ins);
    void execute_lui(std::uint32_t ins);
    void execute_auipc(std::uint32_t ins);
    void execute_ecall(std::uint32_t ins);
    void execute_ebreak(std::uint32_t ins);
    void execute_fence(std::uint32_t ins);
    void execute_na(std::uint32_t ins);

private:
    Mem<1024 * 1024> mem{}; // 1MB of memory

    friend CPUAccessor;
};