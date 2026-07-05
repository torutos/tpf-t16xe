#include "t16xe.hpp"

uint32_t t16xe::phys_addr(const uint8_t seg, const uint16_t offset) {
        if (offset >= 0xFFF0) {
            return offset; // порты
        }

        return (static_cast<uint32_t>(seg) << 16) | offset;
    }

uint8_t t16xe::decode_reg8() { return memory[phys_addr(CS, PC++)]; }

uint16_t t16xe::decode_reg() { return memory[phys_addr(CS, PC++)]; }

uint16_t t16xe::decode_imm16()
{
    const auto lo = memory[phys_addr(CS, PC++)];
    const auto hi = memory[phys_addr(CS, PC++)];
    return (hi << 8) | lo;
}

int8_t t16xe::decode_imm8()
{
    return static_cast<int8_t>(memory[phys_addr(CS, PC++)]);
}

void t16xe::write_reg(const uint16_t reg, const uint16_t val)
{
    switch (reg)
    {
    case REG_AX:
        AX = val;
        break;
    case REG_BX:
        BX = val;
        break;
    case REG_CX:
        CX = val;
        break;
    case REG_DX:
        DX = val;
        break;
    case REG_SP:
        SP = val;
        break;
    default:
        break;
    }
}
uint16_t t16xe::read_reg(const uint16_t reg) const
{
    switch (reg)
    {
    case REG_AX:
        return AX;
    case REG_BX:
        return BX;
    case REG_CX:
        return CX;
    case REG_DX:
        return DX;
    case REG_SP:
        return SP;
    default:
        return 0;
    }
}

void t16xe::update_flags(uint16_t val)
{
    flagZ = (val == 0);
    flagN = (val & 0x8000) != 0;
}
