#pragma once

#include <cstdint>

constexpr std::size_t kRDstartBit = 7;
constexpr std::size_t kRDendBit = 11;

constexpr std::size_t kRS1startBit = 15;
constexpr std::size_t kRS1endBit = 19;

constexpr std::size_t kRS2startBit = 20;
constexpr std::size_t kRS2endBit = 24;

constexpr std::uint64_t bit_mask(std::uint64_t bits)
{
    return bits == 64 ? ~0ULL : (1ULL << bits) - 1;
}

constexpr std::uint64_t extract_count(std::uint64_t value, std::uint64_t start, std::uint64_t bits)
{
    return (value >> start) & bit_mask(bits);
}

constexpr std::uint64_t extract(std::uint64_t value, std::uint64_t start, std::uint64_t end)
{
    return extract_count(value, start, end - start + 1);
}

constexpr std::uint64_t extract(std::uint64_t value, std::uint64_t position)
{
    return extract_count(value, position, 1);
}

constexpr std::uint8_t extract_rd(std::uint32_t instruction)
{
    return static_cast<std::uint8_t>(extract(instruction, kRDstartBit, kRDendBit));
}

constexpr std::uint8_t extract_rs1(std::uint32_t instruction)
{
    return static_cast<std::uint8_t>(extract(instruction, kRS1startBit, kRS1endBit));
}

constexpr std::uint8_t extract_rs2(std::uint32_t instruction)
{
    return static_cast<std::uint8_t>(extract(instruction, kRS2startBit, kRS2endBit));
}

constexpr std::uint32_t extract_opcode(std::uint32_t instruction)
{
    return static_cast<std::uint32_t>(extract(instruction, 0, 6));
}

constexpr std::uint32_t extract_funct3(std::uint32_t instruction)
{
    return static_cast<std::uint32_t>(extract(instruction, 12, 14));
}

constexpr std::uint32_t extract_funct7(std::uint32_t instruction)
{
    return static_cast<std::uint32_t>(extract(instruction, 25, 31));
}

constexpr std::int64_t sign_extend(std::uint64_t value, std::size_t bits)
{
    std::uint64_t shift = 64 - bits;
    return static_cast<std::int64_t>(value << shift) >> shift;
}

constexpr std::uint16_t extract_i_type_imm(std::uint32_t instruction)
{
    return static_cast<std::uint16_t>(extract(instruction, 20, 31));
}

constexpr std::uint16_t extract_s_type_imm(std::uint32_t instruction)
{
    auto imm4_0 = extract(instruction, 7, 11);
    auto imm11_5 = extract(instruction, 25, 31);
    return static_cast<std::uint16_t>((imm11_5 << 5) | imm4_0);
}

constexpr std::uint16_t extract_b_type_imm(std::uint32_t instruction)
{
    auto imm_1_4 = extract(instruction, 8, 11);
    auto imm_5_10 = extract(instruction, 25, 30);
    auto imm_11 = extract(instruction, 7);
    auto imm_12 = extract(instruction, 31);

    return static_cast<std::uint16_t>((imm_12 << 12) | (imm_11 << 11) | (imm_5_10 << 5) | (imm_1_4 << 1));
}

constexpr std::uint32_t extract_u_type_imm(std::uint32_t instruction)
{
    return static_cast<std::uint32_t>(extract(instruction, 12, 31) << 12);
}

constexpr std::uint32_t extract_j_type_imm(std::uint32_t instruction)
{
    auto imm1_10 = extract(instruction, 21, 30);  // bits 30:21
    auto imm11 = extract(instruction, 20);        // bit 20
    auto imm12_19 = extract(instruction, 12, 19); // bits 19:12
    auto imm20 = extract(instruction, 31);        // bit 31

    return static_cast<std::uint32_t>((imm20 << 20) | (imm12_19 << 12) | (imm11 << 11) | (imm1_10 << 1));
}