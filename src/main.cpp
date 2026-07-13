#include <print>
#include <fstream>
#include <vector>
#include <cstddef>

#include "cpu.h"

int main(int argc, char **argv)
{
    std::ifstream prog("program.bin", std::ios::binary);
    if (!prog)
    {
        std::println("Failed to open program.bin");
        return 1;
    }

    std::vector<char> buffer(std::istreambuf_iterator<char>(prog), {});
    prog.close();

    CPU *cpu = new CPU();
    cpu->write_mem(0, buffer.data(), buffer.size());

    try
    {
        auto ins = cpu->fetch_instruction();
        std::println("Decoded instruction: {}", ins.to_string());
    }
    catch (const std::exception &e)
    {
        std::println("Error fetching instruction: {}", e.what());
    }

    // Dump the first 16 bytes of memory for debugging
    std::println("\nMemory dump (first 16 bytes):");
    for (std::size_t i = 0; i < 16; ++i)
    {
        std::uint8_t byte = cpu->read_mem<std::uint8_t>(i);
        std::print("0x{:02x} ", byte);
    }

    // Dump first 8 registers for debugging
    std::println("\n\nRegister dump (first 8 registers):");
    for (std::size_t i = 0; i < 8; ++i)
    {
        std::uint64_t reg_value = cpu->get_register(static_cast<RegisterIdx>(i));
        std::println("x{}: 0x{:016x}", i, reg_value);
    }

    return 0;
}
