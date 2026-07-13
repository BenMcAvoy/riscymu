#pragma once

#include <array>

#include "ins.h"
#include "mem.h"

class CPU
{
public:
    CPU();

    Instruction fetch_instruction() const;

    void write_mem(std::uint64_t addr, const void *data, std::size_t size)
    {
        mem.write(addr, data, size);
    }

private:
    // x0-x31
    std::array<std::uint64_t, 32> general_registers;
    std::uintptr_t pc; // sp reg for the program counter

    Mem<1024 * 1024> mem; // 1MB of memory
};