// main.cpp — T16XE Emulator + Assembler
#include <iostream>
#include <string>
#include "tasm.hpp"
#include "t16xe.hpp"
#include <fstream>
#include "tools_t16xe.hpp"

void print_usage() {
    std::cout << "T16XE Toolchain\n\n";
    std::cout << "Usage:\n";
    std::cout << "  t16xe <file.toru>          - run .toru file\n";
    std::cout << "  t16xe -asm <file.asm>      - assemble .asm to .toru\n";
    std::cout << "  t16xe -run <file.asm>      - assemble and run .asm\n";
    std::cout << "  t16xe -tasm <file.asm>     - assemble only (alias)\n";
}

int main2(int argc, char* argv[]) {


    if (argc < 2) {
        print_usage();
        return 1;
    }

    std::string mode = argv[1];
    
    // Режим ассемблера
    if (mode == "-asm" || mode == "-tasm") {
        if (argc < 3) {
            std::cerr << "Error: no input file\n";
            return 1;
        }
        std::string input = argv[2];
        std::string output = input.substr(0, input.find_last_of('.')) + ".toru";
        
        // Проверяем -o флаг
        for (int i = 3; i < argc; i++) {
            if (std::string(argv[i]) == "-o" && i + 1 < argc) {
                output = argv[++i];
            }
        }
        
        std::cout << "Assembling " << input << " -> " << output << "\n";
        tasm assembler;
        assembler.assemble_file(input);
        assembler.write_toru(output);
        std::cout << "Done.\n";
        return 0;
    }
    
    // Режим ассемблирования и запуска
    if (mode == "-run") {
        if (argc < 3) {
            std::cerr << "Error: no input file\n";
            return 1;
        }
        std::string input = argv[2];
        std::string toru = input.substr(0, input.find_last_of('.')) + ".toru";
        
        // Ассемблируем
        std::cout << "Assembling " << input << " -> " << toru << "\n";
        tasm assembler;
        assembler.assemble_file(input);
        assembler.write_toru(toru);
        
        // Запускаем
        std::cout << "Running " << toru << "...\n";
        t16xe cpu;
        load_toru(cpu, toru);
        cpu.reset();
        cpu.run();
        return 0;
    }
    
    // Режим запуска .toru
    std::string toru = mode;  // первый аргумент — имя файла
    std::cout << "Running " << toru << "...\n";
    t16xe cpu;
    load_toru(cpu, toru);
    cpu.reset();
    cpu.run();
    
    return 0;
}
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: t16xe <file.asm>\n";
        return 1;
    }

    std::string input = argv[1];
    std::string toru = input.substr(0, input.find_last_of('.')) + ".toru";
    
    tasm t;
    t.assemble_file(input);
    t.write_toru(toru);
    
    t16xe cpu;
    load_toru(cpu, toru);
    
    cpu.reset();
    cpu.run();
    
    return 0;
}