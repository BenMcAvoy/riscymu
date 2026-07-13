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
        std::print("Failed to open program.bin\n");
        return 1;
    }

    std::vector<char> buffer(std::istreambuf_iterator<char>(prog), {});
    prog.close();

    CPU *cpu = new CPU();
    cpu->write_mem(0, buffer.data(), buffer.size());

    try
    {
        auto ins = cpu->fetch_instruction();

        std::string ins_str = ins.to_string();
        std::print("Fetched instruction: {}\n", ins_str);
    }
    catch (const std::exception &e)
    {
        std::print("Error fetching instruction: {}\n", e.what());
    }

    return 0;
}
