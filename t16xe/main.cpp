#include <iostream>
#include <string>
#include "t16xe.hpp"
#include "tools.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "T16XE - Toru16 Emulator\n";
        std::cout << "Usage: t16xe -r <file.toru> [-h]\n";
        return 1;
    }

    std::string file;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-h") {
            std::cout << "T16XE - Toru16 Emulator\n";
            std::cout << "Usage: t16xe -r <file.toru>\n";
            return 0;
        } else if (arg == "-r" && i + 1 < argc) {
            file = argv[++i];
        }
    }

    if (file.empty()) {
        std::cerr << "Error: no input file\n";
        return 1;
    }

    t16xe cpu;
    load_toru(cpu, file);
    cpu.reset();
    cpu.run();
    return 0;
}