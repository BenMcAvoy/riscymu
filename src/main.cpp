#include <print>
#include <fstream>
#include <vector>
#include <cstddef>

#include "cpu.h"

extern void platform_setup();

int main(int argc, char **argv)
{
    platform_setup();

    std::ifstream prog("program.bin", std::ios::binary);
    if (!prog)
    {
        std::println("Failed to open program.bin");
        return 1;
    }

    std::vector<char> buffer(std::istreambuf_iterator<char>(prog), {});
    prog.close();

    CPU *cpu = new CPU();
    cpu->warm();
    cpu->write_mem(0, buffer.data(), buffer.size());

    Instruction ins;
    for (int i = 0; i < 1000000; ++i)
    {
        cpu->reset(false);

        while (cpu->step())
        {
        }
    }

    // Dump the first 16 bytes of memory for debugging
    std::println("\nMemory dump (first 16 bytes):");
    for (std::size_t i = 0; i < 16; ++i)
    {
        std::uint8_t byte = cpu->read_mem<std::uint8_t>(i);
        std::print("0x{:02x} ", byte);
    }

    // Dump first 10 registers for debugging
    std::println("\n\nRegister dump (first 10 registers):");
    for (std::size_t i = 0; i < 10; ++i)
    {
        std::uint64_t reg_value = cpu->get_register(static_cast<RegisterIdx>(i));
        std::println("x{}: 0x{:016x}", i, reg_value);
    }

    std::println("\nPerf: {} MHz", cpu->get_mhz());

    return 0;
}
