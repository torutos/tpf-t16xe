#pragma once
#include "t16xe.hpp"
#include <fstream>
int load_toru(t16xe& cpu, const std::string& path) {
    std::ifstream file(path, std::ios::binary);

    uint16_t dsize = file.get() | (file.get() << 8);
    uint16_t csize = file.get() | (file.get() << 8);
    uint16_t code_start = file.get() | (file.get() << 8); // org_addr из заголовка
    
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

    
    cpu.memory[0xFFF4] = code_start & 0xFF;
    cpu.memory[0xFFF5] = code_start >> 8;
    cpu.memory[0xFFF6] = 0x00;
    cpu.memory[0xFFF7] = 0x00;

    return dsize + csize;
}