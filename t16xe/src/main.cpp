// main.cpp — T16XE Emulator + Assembler
#include <iostream>
#include <string>
#include "asm/tasm.hpp"
#include "kernel/t16xe.hpp"
#include <fstream>
#include "kernel/tools.hpp"

void print_usage()
{
    std::cout << "T16XE Toolchain\n\n";
    std::cout << "Usage:\n";
    std::cout << "  t16xe <file.toru>          - run .toru file\n";
    std::cout << "  t16xe -asm <file.asm>      - assemble .asm to .toru\n";
    std::cout << "  t16xe -run <file.asm>      - assemble and run .asm\n";
    std::cout << "  t16xe -tasm <file.asm>     - assemble only (alias)\n";
}

int main(const int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: t16xe <file.asm>\n";
        return 1;
    }

    const std::string input = argv[1];
    const std::string toru = input.substr(0, input.find_last_of('.')) + ".toru";
    

    tasm t;
    t.assemble_file(input);
    t.write_toru(toru);
    t16xe cpu;
    load_toru(cpu, toru);

    cpu.reset();
    cpu.run();

    return 0;
}