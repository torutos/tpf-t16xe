#pragma once
#include "t16xe.hpp"
#include <fstream>

int print16(std::string str, t16xe& cpu, int addr) {

    for (auto el : str) {
        cpu.memory[addr++] = MOV_REG_IMM;
        cpu.memory[addr++] = REG_AX;
        cpu.memory[addr++] = el;
        cpu.memory[addr++] = 0x00;

        cpu.memory[addr++] = MOV_MEM_REG;
        cpu.memory[addr++] = 0xF2;
        cpu.memory[addr++] = 0xFF;
        cpu.memory[addr++] = REG_AX;
    }
    return addr; // следующий свободный адрес
}

int load_toru(t16xe& cpu, const std::string& path) {
    std::ifstream file(path, std::ios::binary);

    uint16_t dsize = file.get() | (file.get() << 8);
    uint16_t csize = file.get() | (file.get() << 8);
    uint16_t code_start = file.get() | (file.get() << 8); // org_addr из заголовка
    // std::cout << "load_toru: dsize=" << dsize << " csize=" << csize << " code_start=" << code_start << "\n";
    // Данные загружаем в 0x10000
    uint32_t data_addr = 0x10000;
    for (int i = 0; i < dsize; i++)
        cpu.memory[data_addr + i] = file.get();

    // Код загружаем по адресу из заголовка
    int loaded = 0;
    for (int i = 0; i < csize; i++) {
        int b = file.get();
        if (b == EOF) {
            // std::cerr << "EOF at code byte " << i << " (csize=" << csize << ")\n";
            break;
        }
        cpu.memory[code_start + i] = b;
        loaded++;
    }

    // Вектор сброса на code_start
    cpu.memory[0xFFF4] = code_start & 0xFF;
    cpu.memory[0xFFF5] = code_start >> 8;
    cpu.memory[0xFFF6] = 0x00;
    cpu.memory[0xFFF7] = 0x00;

    // std::cout << "Code around start (0x" << std::hex << code_start + (1100 - 1024)
    //     << "): ";
    // for (int i = -4; i <= 8; i++) {
    //     std::cout << std::hex << (int)cpu.memory[1100 + i] << " ";
    // }
    // std::cout << "\n";

    return dsize + csize;
}