#include "t16xe.hpp"

uint32_t t16xe::phys_addr(uint16_t seg, uint16_t offset)
{
    uint32_t addr = ((uint32_t)seg << 16) | offset;
    if (addr >= 0x1FFFFF0) return addr; // порты
    return addr;
}


uint16_t t16xe::decode_reg() { return memory[phys_addr(CS, PC++)]; }

uint16_t t16xe::decode_imm16()
{
    const auto lo = memory[phys_addr(CS, PC++)];
    const auto hi = memory[phys_addr(CS, PC++)];
    return (hi << 8) | lo;
}

uint32_t t16xe::decode_imm32() {
    uint32_t lo = memory[phys_addr(CS, PC++)];
    uint32_t mid1 = memory[phys_addr(CS, PC++)];
    uint32_t mid2 = memory[phys_addr(CS, PC++)];
    uint32_t hi = memory[phys_addr(CS, PC++)];
    return lo | (mid1 << 8) | (mid2 << 16) | (hi << 24);
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

void t16xe::push16(uint16_t val)
{
    SP -= 2;
    auto addr = phys_addr(SS, SP);
    memory[addr] = val & 0xFF;
    memory[addr + 1] = val >> 8;
}

uint16_t t16xe::pop16()
{
    auto addr = phys_addr(SS, SP);
    auto lo = memory[addr];
    auto hi = memory[addr + 1];
    uint16_t value =(hi << 8) | lo;
    SP += 2;
    return value;
}

uint16_t t16xe::pack_flags() const
{
    uint16_t f = 0;
    if (flagC) f |= 1;
    if (flagZ) f |= 2;
    if (flagN) f |= 4;
    if (flagO) f |= 8;
    if (flagI) f |= 16;
    return f;
}

void t16xe::unpack_flags(uint16_t f)
{
    flagC = f & 1;
    flagZ = f & 2;
    flagN = f & 4;
    flagO = f & 8;
    flagI = f & 16;
}