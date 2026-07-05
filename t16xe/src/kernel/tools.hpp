#pragma once
#include "t16xe.hpp"
#include <fstream>

inline int load_toru(t16xe& cpu, const std::string& path) {
    std::ifstream file(path, std::ios::binary);

    const uint16_t dsize = file.get() | (file.get() << 8);
    const uint16_t csize = file.get() | (file.get() << 8);
    const uint16_t code_start = file.get() | (file.get() << 8);

    constexpr uint32_t data_addr = 0x10000;
    file.read(reinterpret_cast<char*>(&cpu.memory[data_addr]), dsize);
    file.read(reinterpret_cast<char*>(&cpu.memory[code_start]), csize);

    cpu.memory[0xFFF4] = code_start & 0xFF;
    cpu.memory[0xFFF5] = code_start >> 8;
    cpu.memory[0xFFF6] = 0x00;
    cpu.memory[0xFFF7] = 0x00;

    return dsize + csize;
}