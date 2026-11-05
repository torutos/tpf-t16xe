#include "t16xe.hpp"

bool t16xe::execute(uint8_t opcode)
{
    PC++;

    switch (opcode)
    {
    case HLT:
        {
            return false; // останов run()
        }

    case MOV_REG_IMM:
        {
            auto reg = decode_reg(); // читаем код регистра
            auto val = decode_imm32(); // читаем 16-битное значение
            write_reg(reg, val);
            break;
        }
    case MOV_REG_REG:
        {
            auto dst = decode_reg();
            auto src = decode_reg();
            auto val = read_reg(src);
            write_reg(dst, val);
            break;
        }

    case ADD_REG_REG:
        {
            auto dst = decode_reg();
            auto src = decode_reg();
            uint16_t a = read_reg(dst);
            uint16_t b = read_reg(src);
            uint32_t full_result = (uint32_t)a + b; // Считаем в 32 битах
            uint16_t result = (uint16_t)(full_result & 0xFFFF); // Обрезаем для записи
            write_reg(dst, result);
            update_flags(result);
            flagC = (full_result > 0xFFFF); // перенос
            flagO = ((a ^ result) & (b ^ result) & 0x8000) != 0;
            break;
        }

    case SUB_REG_REG:
        {
            auto dst = decode_reg();
            auto src = decode_reg();
            uint16_t a = read_reg(dst);
            uint16_t b = read_reg(src);
            uint32_t full_sub = (uint32_t)a - b;
            uint16_t result = (uint16_t)(full_sub & 0xFFFF);
            write_reg(dst, result);
            update_flags(result);
            flagC = (full_sub > 0xFFFF); // заём
            flagO = ((a ^ b) & (a ^ result) & 0x8000) != 0;
            break;
        }

    case CMP_REG_REG:
        {
            auto a_reg = decode_reg();
            auto b_reg = decode_reg();
            uint16_t a = read_reg(a_reg);
            uint16_t b = read_reg(b_reg);
            uint32_t full_sub = (uint32_t)a - b;
            uint16_t result = (uint16_t)(full_sub & 0xFFFF);
            update_flags(result);
            flagC = (full_sub > 0xFFFF);
            flagO = ((a ^ b) & (a ^ result) & 0x8000) != 0;
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
            auto off = decode_imm32();
            uint16_t new_cs = (off >> 16) & 0xFFFF;
            uint16_t new_pc = off & 0xFFFF;

            CS = new_cs;
            PC = new_pc;
            break;
        }

    case PUSH_REG:
        {
            auto reg = decode_reg();
            push16(read_reg(reg));
            break;
        }

    case POP_REG:
        {
            auto reg = decode_reg();
            write_reg(reg, pop16());
            break;
        }

    case MOV_REG_MEM:
        {
            auto reg = decode_reg();
            auto off = decode_imm32();
            auto addr = phys_addr(DS, off);
            auto lo = memory[addr];
            auto hi = memory[addr + 1];
            auto val = (hi << 8) | lo;
            write_reg(reg, val);
            break;
        }

    case MOV_MEM_REG:
        {
            auto off = decode_imm32();
            auto reg = decode_reg();
            auto val = read_reg(reg);
            if (off == 0x1FFFFF2)
            {
                if (output_callback) output_callback(static_cast<char>(val & 0xFF));
                else std::cout << static_cast<char>(val & 0xFF);
                std::cout.flush();
                break;
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
            flagC = (val == 0);
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
            break;
        }

    case AND_REG_REG:
        {
            auto dst = decode_reg();
            auto src = decode_reg();
            auto result = read_reg(dst) & read_reg(src);
            write_reg(dst, result);
            update_flags(result);
            flagC = false;
            flagO = false;
            break;
        }
    case OR_REG_REG:
        {
            auto dst = decode_reg();
            auto src = decode_reg();
            auto result = read_reg(dst) | read_reg(src);
            write_reg(dst, result);
            update_flags(result);
            flagC = false;
            flagO = false;
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
            flagO = false;
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
            flagC = (shift > 0) ? ((val & (1 << (16 - shift))) != 0) : false; // последний выдвинутый бит
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
            flagC = (shift > 0) ? ((val & (1 << (shift - 1))) != 0) : false; // последний выдвинутый бит
            break;
        }
    case CALL:
        {
            auto off = decode_imm32();
            uint16_t new_cs = (off >> 16) & 0xFFFF;
            uint16_t new_pc = off & 0xFFFF;
            push16(CS);
            push16(PC);

            CS = new_cs;
            PC = new_pc;
            break;
        }
    case RET:
        {
            PC = pop16();
            CS = pop16();
            break;
        }
    case ADD_REG_IMM:
        {
            auto reg = decode_reg();
            uint16_t a = read_reg(reg);
            uint32_t imm = decode_imm32();
            uint32_t full_result = (uint32_t)a + imm;
            auto result = (uint16_t)(full_result & 0xFFFF);
            write_reg(reg, result);
            update_flags(result);
            flagC = (full_result > 0xFFFF); // <--- ВОТ ТАК
            flagO = ((a ^ result) & (imm ^ result) & 0x8000) != 0;
            break;
        }
    case SUB_REG_IMM:
        {
            auto reg = decode_reg();
            uint16_t a = read_reg(reg);
            uint32_t imm = decode_imm32();
            uint32_t full_sub = (uint32_t)a - imm;
            auto result = (uint16_t)(full_sub & 0xFFFF);
            write_reg(reg, result);
            update_flags(result);
            flagC = (full_sub > 0xFFFF); // или (a >= imm)
            flagO = ((a ^ imm) & (a ^ result) & 0x8000) != 0;
            break;
        }
    case CMP_REG_IMM:
        {
            auto reg = decode_reg();
            uint16_t a = read_reg(reg);
            uint32_t imm = decode_imm32();
            uint32_t full_sub = (uint32_t)a - imm;
            uint16_t result = (uint16_t)(full_sub & 0xFFFF);
            update_flags(result);
            flagC = (full_sub > 0xFFFF);
            flagO = ((a ^ imm) & (a ^ result) & 0x8000) != 0;
            break;
        }
    case AND_REG_IMM:
        {
            auto reg = decode_reg();
            auto imm = decode_imm32();
            auto result = read_reg(reg) & imm;
            write_reg(reg, result);
            update_flags(result);
            flagC = false;
            break;
        }
    case OR_REG_IMM:
        {
            auto reg = decode_reg();
            auto imm = decode_imm32();
            auto result = read_reg(reg) | imm;
            write_reg(reg, result);
            update_flags(result);
            flagC = false;
            break;
        }
    case XOR_REG_IMM:
        {
            auto reg = decode_reg();
            auto imm = decode_imm32();
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

            // Сохраняем контекст (флаги, CS, PC) — по 2 байта!
            push16(pack_flags());
            push16(CS);
            push16(PC);

            flagI = true; // Запрещаем прерывания

            // Читаем вектор из таблицы (база 0x10000)
            uint32_t vec_addr = 0x10000 + vector * 4;
            PC = memory[vec_addr] | (memory[vec_addr + 1] << 8);
            CS = memory[vec_addr + 2] | (memory[vec_addr + 3] << 8);

            break;
        }

    case IRET:
        {
            PC = pop16();
            CS = pop16();
            unpack_flags(pop16());
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
            auto off = decode_imm32();
            auto addr = phys_addr(DS, off);
            write_reg(reg, memory[addr]);
            break;
        }
    case STB_MEM_REG:
        {
            auto off = decode_imm32();
            auto reg = decode_reg();
            auto val = read_reg(reg) & 0xFF;
            if (off == 0x1FFFFF2)
            {
                if (output_callback) output_callback(static_cast<char>(val & 0xFF));
                else std::cout << static_cast<char>(val & 0xFF);
                std::cout.flush();
                break;
            }
            auto addr = phys_addr(DS, off);
            memory[addr] = val;
            break;
        }
    case MOV_MEMREG_REG:
        {
            // 0x25
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
    case MUL_REG_REG:
        {
            auto src = decode_reg();
            uint32_t result = (uint32_t)AX * read_reg(src);
            DX = result >> 16;
            AX = result & 0xFFFF;
            update_flags(AX);
            flagC = (DX != 0);
            break;
        }
    case DIV_REG_REG:
        {
            auto dst = decode_reg();
            auto src = decode_reg();
            uint16_t divisor = read_reg(src);
            if (divisor == 0)
            {
                flagC = true;
                break;
            }
            uint16_t result = read_reg(dst) / divisor;
            write_reg(dst, result);
            update_flags(result);
            flagC = false;
            break;
        }
    case NOT_REG:
        {
            auto reg = decode_reg();
            auto val = ~read_reg(reg);
            write_reg(reg, val);
            update_flags(val);
            break;
        }
    case NEG_REG:
        {
            auto reg = decode_reg();
            auto val = -read_reg(reg);
            write_reg(reg, val);
            update_flags(val);
            flagC = (val != 0);
            break;
        }
    case PUSHF:
        {
            push16(pack_flags());
            break;
        }
    case POPF:
        {
            unpack_flags(pop16());
            break;
        }
    case ADC_REG_REG:
        {
            auto dst = decode_reg();
            auto src = decode_reg();
            uint16_t a = read_reg(dst);
            uint16_t b = read_reg(src);
            uint32_t carry = flagC ? 1 : 0;
            uint32_t full_result = (uint32_t)a + b + carry;
            auto result = (uint16_t)(full_result & 0xFFFF);
            write_reg(dst, result);
            update_flags(result);
            flagC = (full_result > 0xFFFF);
            // flagO для ADC считается так же, как для ADD
            flagO = ((a ^ result) & (b ^ result) & 0x8000) != 0;
            break;
        }
    case SBB_REG_REG:
        {
            auto dst = decode_reg();
            auto src = decode_reg();
            uint16_t a = read_reg(dst);
            uint16_t b = read_reg(src);
            uint32_t borrow = flagC ? 1 : 0;
            uint32_t full_sub = (uint32_t)a - b - borrow;
            auto result = (uint16_t)(full_sub & 0xFFFF);
            write_reg(dst, result);
            update_flags(result);
            // Если full_sub ушёл в минус (в 32-битном беззнаковом это огромное число), значит был заём
            flagC = (full_sub > 0xFFFF); // или (a < b + borrow), что то же самое
            flagO = ((a ^ b) & (a ^ result) & 0x8000) != 0;
            break;
        }
    case TEST_REG_REG:
        {
            auto a = read_reg(decode_reg());
            auto b = read_reg(decode_reg());
            auto result = a & b;
            update_flags(result);
            flagC = false;
            flagO = false;
            break;
        }
    case JS:
        {
            auto offset = decode_imm8();
            if (flagN) PC += offset;
            break;
        }
    case JNS:
        {
            auto offset = decode_imm8();
            if (!flagN) PC += offset;
            break;
        }
    case NOP:
        {
            break;
        }
    case MOV_SEG_REG:
        {
            auto seg = decode_reg(); // какой сегмент (0=CS, 1=DS, 2=SS)
            auto reg = decode_reg(); // из какого регистра
            uint16_t val = read_reg(reg);
            switch (seg)
            {
            case 0: CS = val;
                break;
            case 1: DS = val;
                break;
            case 2: SS = val;
                break;
            }
            break;
        }
    case MOV_REG_SEG:
        {
            auto reg = decode_reg(); // куда
            auto seg = decode_reg(); // откуда
            uint16_t val = 0;
            switch (seg)
            {
            case 0: val = CS;
                break;
            case 1: val = DS;
                break;
            case 2: val = SS;
                break;
            }
            write_reg(reg, val);
            break;
        }
    default:
        {
            std::cerr << "Runtime error: unknown opcode 0x" << std::hex << static_cast<int>(opcode)
                << " at PC=0x" << (PC - 1) << "\n";
            return false;
        }
    }

    return true;
}
