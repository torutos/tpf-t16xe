#pragma once
#include "t16xe.hpp"
#include <fstream>

inline int load_toru(t16xe& cpu, const std::string& path) {
    std::ifstream file(path, std::ios::binary);

    uint32_t dsize = file.get() | (file.get() << 8) | (file.get() << 16) | (file.get() << 24);
    uint32_t csize = file.get() | (file.get() << 8) | (file.get() << 16) | (file.get() << 24);
    uint32_t code_start = file.get() | (file.get() << 8) | (file.get() << 16) | (file.get() << 24);
    uint32_t data_start = file.get() | (file.get() << 8) | (file.get() << 16) | (file.get() << 24);

    file.read(reinterpret_cast<char*>(&cpu.memory[data_start]), dsize);
    file.read(reinterpret_cast<char*>(&cpu.memory[code_start]), csize);

    cpu.memory[0x1FFFFF4] = code_start & 0xFF;
    cpu.memory[0x1FFFFF5] = (code_start >> 8) & 0xFF;
    cpu.memory[0x1FFFFF6] = (code_start >> 16) & 0xFF;
    cpu.memory[0x1FFFFF7] = (code_start >> 24) & 0xFF;

    return dsize + csize;
}