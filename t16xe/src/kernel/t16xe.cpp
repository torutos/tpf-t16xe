#include "t16xe.hpp"

void t16xe::reset()
{
    AX = BX = CX = DX = 0;
    PC = 0;
    SP = 0xFFFE;
    CS = 0;
    DS = 0x01;
    SS = 0;
    flagC = flagZ = flagN = flagO = false;
    flagI = true; // прерывания запрещены

    // Вектор сброса: читаем 4 байта из конца памяти
    const uint32_t vec = phys_addr(0, 0xFFF4); // CS=0, offset=0xFFF0
    const uint16_t lo = memory[vec] | (memory[vec + 1] << 8);
    const uint16_t hi = memory[vec + 2] | (memory[vec + 3] << 8);
    CS = hi; // сегмент
    PC = lo; // смещение
}

bool t16xe::step()
{
    if (!flagI)
    {
        if (memory[0xFFF0] & 1)
        {
            const uint8_t flags = (flagC ? 1 : 0) | (flagZ ? 2 : 0) | (flagN ? 4 : 0) |
                            (flagO ? 8 : 0) | (flagI ? 16 : 0);
            SP -= 2;
            auto addr = phys_addr(SS, SP);
            memory[addr] = flags;
            memory[addr + 1] = 0;

            SP -= 2;
            addr = phys_addr(SS, SP);
            memory[addr] = CS;
            memory[addr + 1] = 0;

            SP -= 2;
            addr = phys_addr(SS, SP);
            memory[addr] = PC & 0xFF;
            memory[addr + 1] = PC >> 8;

            flagI = true;

            uint32_t vec_addr = 0x10000 + 0x09 * 4;
            uint16_t new_ip = memory[vec_addr] | (memory[vec_addr + 1] << 8);
            uint8_t new_cs = memory[vec_addr + 2];

            CS = new_cs;
            PC = new_ip;
            memory[0xFFF0] = 0;
            return true;
        }
    }

    const uint32_t addr = phys_addr(CS, PC);

    if (const uint8_t opcode = memory[addr]; !execute(opcode))
        return false;

    if (breakpoints.count(PC) && !breakpoint_hit)
    {
        breakpoint_hit = true;
        return false;
    }
    breakpoint_hit = false;
    
    return true;
}

void t16xe::cont()
{
    breakpoint_hit = false;
    while (step()) {}
}

void t16xe::run()
{
    while (step()) {}
}