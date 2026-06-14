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
    uint32_t vec = phys_addr(0, 0xFFF4); // CS=0, offset=0xFFF0
    uint16_t lo = memory[vec] | (memory[vec + 1] << 8);
    uint16_t hi = memory[vec + 2] | (memory[vec + 3] << 8);
    CS = hi; // сегмент
    PC = lo; // смещение
}

void t16xe::run()
{
    // std::cout << "RUN START\n";
    while (true)
    {
        // Проверяем прерывани
        // std::cout << "flagI=" << flagI << "\n";
        if (!flagI)
        {
            // std::cout << "flagI=0, kbd=" << std::hex << (int)memory[0xFFF0] <<
            // "\n";
            if (memory[0xFFF0] & 1)
            {
                // std::cout << "IRQ!\n";
                // std::cout << "IRQ at PC=" << PC << "\n";
                // Вызываем INT 0x09 (клавиатура)
                // Сохраняем флаги
                uint8_t flags = (flagC ? 1 : 0) | (flagZ ? 2 : 0) | (flagN ? 4 : 0) |
                                (flagO ? 8 : 0) | (flagI ? 16 : 0);
                SP -= 2;
                auto addr = phys_addr(SS, SP);
                memory[addr] = flags;
                memory[addr + 1] = 0;

                // Сохраняем CS
                SP -= 2;
                addr = phys_addr(SS, SP);
                memory[addr] = CS;
                memory[addr + 1] = 0;

                // Сохраняем PC (текущий, не меняем)
                SP -= 2;
                addr = phys_addr(SS, SP);
                memory[addr] = PC & 0xFF;
                memory[addr + 1] = PC >> 8;
                // std::cout << "Saved PC: lo=" << std::hex << (int)memory[addr]
                //     << " hi=" << (int)memory[addr + 1] << " PC was " << PC
                //     << "\n";

                // Запрещаем прерывания
                flagI = true;

                // Читаем вектор 0x09
                uint32_t vec_addr = 0x10000 + 0x09 * 4;
                uint16_t new_ip = memory[vec_addr] | (memory[vec_addr + 1] << 8);
                uint8_t new_cs = memory[vec_addr + 2];

                CS = new_cs;
                PC = new_ip;
                // std::cout << "IRQ! jumping to " << new_ip << "\n";
                // Сбрасываем флаг клавиатуры
                memory[0xFFF0] = 0;
                // std::cout << "Saved PC to stack: lo=" << std::hex
                //     << (int)memory[phys_addr(SS, SP)]
                //     << " hi=" << (int)memory[phys_addr(SS, SP) + 1] << "\n";
                continue; // переходим к выполнению обработчика
            }
        }
        // std::cout << "CS=" << (int)CS << " PC=" << PC << "\n";
        // Читаем опкод
        uint32_t addr = phys_addr(CS, PC);
        uint8_t opcode = memory[addr];
        // std::cout << "opcode=" << std::hex << (int)opcode << " at PC=" << PC <<
        // "\n";
        if (!execute(opcode))
            break;
    }
}