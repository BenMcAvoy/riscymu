#include "ins.h"

#include <sstream>
#include <magic_enum.hpp>

std::string Instruction::to_string() const
{
    std::stringstream ss;
    ss << "Instruction(raw=0x" << std::hex << raw << ", format=";
    ss << magic_enum::enum_name(format);
    ss << ", op=" << magic_enum::enum_name(op);
    ss << ", op_group=" << magic_enum::enum_name(op_group);
    ss << ", rd=" << std::dec << static_cast<int>(rd);
    ss << ", rs1=" << std::dec << static_cast<int>(rs1);
    ss << ", rs2=" << std::dec << static_cast<int>(rs2);
    ss << ", funct3=" << std::dec << static_cast<int>(funct3);
    ss << ", funct7=" << std::dec << static_cast<int>(funct7);
    ss << ", imm=" << std::dec << imm;
    ss << ", length=" << std::dec << static_cast<int>(length);
    ss << ")";

    return ss.str();
}