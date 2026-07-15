#pragma once

#include <array>
#include <chrono>

#include "ins.h"
#include "mem.h"

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

class CPU
{
public:
    CPU();

    Instruction fetch_instruction() const;

    void write_mem(std::uint64_t addr, const void *data, std::size_t size)
    {
        mem.write(addr, data, size);
    }

    template <typename T>
    T read_mem(std::uint64_t addr) const
    {
        return mem.read<T>(addr);
    }

    std::uint64_t get_register(RegisterIdx idx)
    {
        return general_registers[static_cast<std::size_t>(idx)];
    }

    void set_register(RegisterIdx idx, std::uint64_t value)
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

    void decode_full_opcode(Instruction &ins, std::int32_t full) const;

    // void decode_full_op(Instruction &ins, std::int32_t full) const;
    // void decode_full_op_imm(Instruction &ins, std::int32_t full) const;
    // void decode_full_load(Instruction &ins, std::int32_t full) const;
    // void decode_full_store(Instruction &ins, std::int32_t full) const;
    // void decode_full_branch(Instruction &ins, std::int32_t full) const;
    // void decode_full_jal(Instruction &ins, std::int32_t full) const;
    // void decode_full_jalr(Instruction &ins, std::int32_t full) const;
    // void decode_full_auipc(Instruction &ins, std::int32_t full) const;
    // void decode_full_lui(Instruction &ins, std::int32_t full) const;
    // void decode_full_etype(Instruction &ins, std::int32_t full) const;
    // void decode_full_fence(Instruction &ins, std::int32_t full) const;

    void fetch_full_instruction(Instruction &ins, std::int32_t full) const;
    void fetch_half_instruction(Instruction &ins, std::int16_t half) const;

    int executed_instructions;
    mutable std::chrono::time_point<std::chrono::high_resolution_clock> start_time;

    Mem<1024 * 1024> mem{}; // 1MB of memory
};