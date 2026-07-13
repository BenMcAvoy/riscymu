#include "ins.h"

#include <sstream>
#include <magic_enum.hpp>

std::string Instruction::to_string() const
{
    std::stringstream ss;
    ss << "Instruction(raw=0x" << std::hex << raw << ", format=";
    ss << magic_enum::enum_name(format);
    ss << ", op=" << magic_enum::enum_name(op);
    ss << ", rd=" << std::dec << static_cast<int>(rd);
    ss << ", rs1=" << std::dec << static_cast<int>(rs1);
    ss << ", rs2=" << std::dec << static_cast<int>(rs2);
    ss << ", imm=" << std::dec << imm;
    ss << ")";

    return ss.str();
}