#include "t16xe.hpp"

bool t16xe::execute(uint8_t opcode)
{
    // std::cout << "exec opcode=" << std::hex << (int)opcode << " at PC=" << PC -
    // 1
    //          << "\n";
    // std::cout << "PC=" << PC << " opcode=" << std::hex << (int)opcode << "\n";
    PC++;

    switch (opcode)
    {
    case HLT:
    {
        return false; // останов run()
    }

    case MOV_REG_IMM:
    {
        auto reg = decode_reg();   // читаем код регистра
        auto val = decode_imm16(); // читаем 16-битное значение
        write_reg(reg, val);
        update_flags(val);
        break;
    }
    case MOV_REG_REG:
    {
        auto dst = decode_reg();
        auto src = decode_reg();
        auto val = read_reg(src);
        write_reg(dst, val);
        update_flags(val);
        break;
    }

    case ADD_REG_REG:
    {
        auto dst = decode_reg();
        auto src = decode_reg();
        auto a = read_reg(dst);
        auto b = read_reg(src);
        auto result = a + b;
        write_reg(dst, result);
        update_flags(result);
        flagC = (result < a);                                // перенос
        flagO = ((a ^ result) & (b ^ result) & 0x8000) != 0; // переполнение
        break;
    }

    case SUB_REG_REG:
    {
        auto dst = decode_reg();
        auto src = decode_reg();
        auto a = read_reg(dst);
        auto b = read_reg(src);
        auto result = a - b;
        write_reg(dst, result);
        update_flags(result);
        flagC = (a >= b); // заём инвертирован
        break;
    }

    case CMP_REG_REG:
    {
        auto a_reg = decode_reg();
        auto b_reg = decode_reg();
        auto a = read_reg(a_reg);
        auto b = read_reg(b_reg);
        auto result = a - b;
        update_flags(result);
        flagC = (a >= b);
        break;
    }

    case JEQ:
    {
        auto offset = decode_imm8(); // знаковое смещение
        if (flagZ)
            PC += offset;
        break;
    }

    case JNE:
    {
        auto offset = decode_imm8();
        if (!flagZ)
            PC += offset;
        break;
    }

    case JMP:
    {
        auto seg = decode_reg8();  // новый CS
        auto off = decode_imm16(); // новый PC
        CS = seg;
        PC = off;
        break;
    }

    case PUSH_REG:
    {
        auto reg = decode_reg();
        auto val = read_reg(reg);
        SP -= 2;
        auto addr = phys_addr(SS, SP);
        memory[addr] = val & 0xFF;
        memory[addr + 1] = val >> 8;
        break;
    }

    case POP_REG:
    {
        auto reg = decode_reg();
        auto addr = phys_addr(SS, SP);
        auto lo = memory[addr];
        auto hi = memory[addr + 1];
        auto val = (hi << 8) | lo;
        SP += 2;
        write_reg(reg, val);
        break;
    }

    case MOV_REG_MEM:
    {
        auto reg = decode_reg();
        auto off = decode_imm16();
        auto addr = phys_addr(DS, off);
        auto lo = memory[addr];
        auto hi = memory[addr + 1];
        auto val = (hi << 8) | lo;
        write_reg(reg, val);
        update_flags(val);
        break;
    }

    case MOV_MEM_REG:
    {
        auto off = decode_imm16();
        auto reg = decode_reg();
        auto val = read_reg(reg);
        // std::cout << "STOR to 0x" << std::hex << off << " val=" << val << "\n";
        if (off == 0xFFF2)
        {
            std::cout << (char)(val & 0xFF);
            std::cout.flush();
        }
        auto addr = phys_addr(DS, off);
        memory[addr] = val & 0xFF;
        memory[addr + 1] = val >> 8;
        break;
    }

    case INC_REG:
    {
        auto reg = decode_reg();
        auto val = read_reg(reg) + 1;
        write_reg(reg, val);
        update_flags(val);
        flagC = (val == 0); // перенос если был переход через FFFF
        break;
    }
    case DEC_REG:
    {
        auto reg = decode_reg();
        auto val = read_reg(reg) - 1;
        write_reg(reg, val);
        update_flags(val);
        flagC = (val != 0xFFFF); // заём если не было underflow
        break;
    }

    case MOV_REG_MEMREG:
    {
        auto dst = decode_reg(); // куда грузим
        auto src = decode_reg(); // в каком регистре адрес
        auto addr = phys_addr(DS, read_reg(src));
        auto lo = memory[addr];
        auto hi = memory[addr + 1];
        auto val = (hi << 8) | lo;
        write_reg(dst, val);
        update_flags(val);
        break;
    }

    case AND_REG_REG:
    {
        auto dst = decode_reg();
        auto src = decode_reg();
        auto result = read_reg(dst) & read_reg(src);
        write_reg(dst, result);
        update_flags(result);
        break;
    }
    case OR_REG_REG:
    {
        auto dst = decode_reg();
        auto src = decode_reg();
        auto result = read_reg(dst) | read_reg(src);
        write_reg(dst, result);
        update_flags(result);
        break;
    }
    case XOR_REG_REG:
    {
        auto dst = decode_reg();
        auto src = decode_reg();
        auto result = read_reg(dst) ^ read_reg(src);
        write_reg(dst, result);
        update_flags(result);
        flagC = false;
        break;
    }
    case SHL_REG_REG:
    {
        auto dst = decode_reg();
        auto src = decode_reg();
        auto val = read_reg(dst);
        auto shift = read_reg(src) & 0x0F; // только младшие 4 бита
        auto result = val << shift;
        write_reg(dst, result);
        update_flags(result);
        flagC = (val & (1 << (16 - shift))) != 0; // последний выдвинутый бит
        break;
    }
    case SHR_REG_REG:
    {
        auto dst = decode_reg();
        auto src = decode_reg();
        auto val = read_reg(dst);
        auto shift = read_reg(src) & 0x0F;
        auto result = val >> shift;
        write_reg(dst, result);
        update_flags(result);
        flagC = (val & (1 << (shift - 1))) != 0; // последний выдвинутый бит
        break;
    }
    case CALL:
    {
        auto seg = decode_reg8();
        auto off = decode_imm16();

        // Сохраняем CS
        SP -= 2;
        auto addr = phys_addr(SS, SP);
        memory[addr] = CS;
        memory[addr + 1] = 0;

        // Сохраняем PC
        SP -= 2;
        addr = phys_addr(SS, SP);
        memory[addr] = PC & 0xFF;
        memory[addr + 1] = PC >> 8;

        // std::cout << "CALL: saved PC=" << PC << " to SP=" << SP << " (mem[" << addr << "]=" << (int)memory[addr] << "," << (int)memory[addr + 1] << ")\n";

        CS = seg;
        PC = off;
        break;
    }
    case RET:
    {
        auto addr = phys_addr(SS, SP);
        PC = memory[addr] | (memory[addr + 1] << 8); // сначала PC
        SP += 2;

        addr = phys_addr(SS, SP);
        CS = memory[addr]; // потом CS
        SP += 2;
        break;
    }
    case ADD_REG_IMM:
    {
        auto reg = decode_reg();
        auto imm = decode_imm16();
        auto result = read_reg(reg) + imm;
        write_reg(reg, result);
        update_flags(result);
        flagC = (result < read_reg(reg));
        break;
    }
    case SUB_REG_IMM:
    {
        auto reg = decode_reg();
        auto imm = decode_imm16();
        auto a = read_reg(reg);
        auto result = a - imm;
        write_reg(reg, result);
        update_flags(result);
        flagC = (a >= imm);
        break;
    }
    case CMP_REG_IMM:
    {
        auto reg = decode_reg();
        auto imm = decode_imm16();
        auto result = read_reg(reg) - imm;
        update_flags(result);
        flagC = (read_reg(reg) >= imm);
        break;
    }
    case AND_REG_IMM:
    {
        auto reg = decode_reg();
        auto imm = decode_imm16();
        auto result = read_reg(reg) & imm;
        write_reg(reg, result);
        update_flags(result);
        flagC = false;
        break;
    }
    case OR_REG_IMM:
    {
        auto reg = decode_reg();
        auto imm = decode_imm16();
        auto result = read_reg(reg) | imm;
        write_reg(reg, result);
        update_flags(result);
        flagC = false;
        break;
    }
    case XOR_REG_IMM:
    {
        auto reg = decode_reg();
        auto imm = decode_imm16();
        auto result = read_reg(reg) ^ imm;
        write_reg(reg, result);
        update_flags(result);
        flagC = false;
        break;
    }

    case LDB_REG_MEMREG:
    {
        auto dst = decode_reg();
        auto src = decode_reg();
        auto addr = phys_addr(DS, read_reg(src));
        write_reg(dst, memory[addr]); // только 1 байт, старший = 0
        update_flags(memory[addr]);
        break;
    }
    case STB_MEMREG_REG:
    {
        auto src = decode_reg(); // регистр с адресом
        auto dst = decode_reg(); // регистр со значением
        auto addr = phys_addr(DS, read_reg(src));
        memory[addr] = read_reg(dst) & 0xFF; // только младший байт
        break;
    }

    case SOFTWARE_INT:
    {
        auto vector = decode_imm8();
        // std::cout << "INT " << (int)vector << " vec_addr=" << vector * 4
        //     << " new_ip="
        //     << (memory[vector * 4] | (memory[vector * 4 + 1] << 8))
        //     << " new_cs=" << (int)memory[vector * 4 + 2] << "\n";
        // Сохраняем флаги в стек
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
        // Сохраняем PC
        SP -= 2;
        addr = phys_addr(SS, SP);
        memory[addr] = PC & 0xFF;
        memory[addr + 1] = PC >> 8;

        // Запрещаем прерывания
        flagI = true;

        // Читаем вектор из таблицы (первые 1024 байта памяти)
        uint32_t vec_addr = vector * 4;
        uint16_t new_ip = memory[vec_addr] | (memory[vec_addr + 1] << 8);
        uint8_t new_cs = memory[vec_addr + 2];

        // Прыгаем на обработчик
        CS = new_cs;
        PC = new_ip;
        break;
    }

    case IRET:
    {
        // std::cout << "HANDLER\n";
        // Восстанавливаем PC
        auto addr = phys_addr(SS, SP);
        PC = memory[addr] | (memory[addr + 1] << 8);
        SP += 2;

        // Восстанавливаем CS
        addr = phys_addr(SS, SP);
        CS = memory[addr];
        SP += 2;

        // Восстанавливаем флаги
        addr = phys_addr(SS, SP);
        uint8_t flags = memory[addr];
        flagC = flags & 1;
        flagZ = flags & 2;
        flagN = flags & 4;
        flagO = flags & 8;
        flagI = flags & 16;
        SP += 2;
        // std::cout << "IRET to PC=" << PC << "\n";
        break;
    }

    case STI:
    {
        flagI = false; // разрешить прерывания
        break;
    }

    case CLI:
    {
        flagI = true; // запретить прерывания
        break;
    }

    case LDB_REG_MEM:
    {
        auto reg = decode_reg();
        auto off = decode_imm16();
        auto addr = phys_addr(DS, off);
        write_reg(reg, memory[addr]);
        update_flags(memory[addr]);
        break;
    }
    case STB_MEM_REG:
    {
        auto off = decode_imm16();
        auto reg = decode_reg();
        auto val = read_reg(reg) & 0xFF;
        if (off == 0xFFF0)
        {
            // std::cout << "SBR: writing " << val << " to " << off << "\n";
        }
        if (off == 0xFFF2)
        {
            std::cout << (char)val;
            std::cout.flush();
        }
        auto addr = phys_addr(DS, off);
        memory[addr] = val;
        break;
    }
    case MOV_MEMREG_REG:
    {                            // 0x25
        auto src = decode_reg(); // регистр с адресом
        auto dst = decode_reg(); // регистр со значением
        auto addr = phys_addr(DS, read_reg(src));
        auto val = read_reg(dst);
        memory[addr] = val & 0xFF;
        memory[addr + 1] = val >> 8;
        break;
    }

    case JCS:
    {
        auto offset = decode_imm8();
        if (flagC)
            PC += offset;
        break;
    }
    case JCC:
    {
        auto offset = decode_imm8();
        if (!flagC)
            PC += offset;
        break;
    }

    default:
    {
        std::cerr << "Runtime error: unknown opcode 0x" << std::hex << (int)opcode
                  << " at PC=0x" << (PC - 1) << "\n";
        return false;
    }
    }

    return true;
}