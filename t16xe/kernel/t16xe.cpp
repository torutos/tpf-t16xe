#include "t16xe.hpp"

void t16xe::reset() {
    AX = BX = CX = DX = 0;
    PC = 0;
    SP = 0xFEEF;
    CS = 0x0001;
    DS = 0x0001;
    SS = 0x0001;
    flagC = flagZ = flagN = flagO = false;
    flagI = true;

    // Вектор сброса в конце 32 МБ
    uint32_t vec = 0x1FFFFF4;
    uint32_t lo = (uint32_t)memory[vec] | ((uint32_t)memory[vec+1] << 8) |
              ((uint32_t)memory[vec+2] << 16) | ((uint32_t)memory[vec+3] << 24);
    CS = (lo >> 16) & 0xFFFF;
    PC = lo & 0xFFFF;
}
bool t16xe::step()
{
    if (input_callback) {
        if (const int c = input_callback(); c >= 0) {
            memory[0xFFF1] = c & 0xFF;   // данные клавиатуры
            memory[0xFFF0] |= 1;          // флаг "есть данные"
        }
    }
    if (!flagI)
    {
        if (memory[0xFFF0] & 1)
        {
            push16(pack_flags());
            push16(CS);
            push16(PC);
            flagI = true;

            uint32_t vec_addr = 0x10000 + 0x09 * 4;
            uint16_t new_ip = memory[vec_addr] | (memory[vec_addr + 1] << 8);
            uint16_t new_cs = memory[vec_addr + 2] | (memory[vec_addr + 3] << 8);

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