#include <iostream>
#include <string>
#include "tasm.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: tasm <file.asm> [-o <output.toru>]\n";
        return 1;
    }

    std::string input = argv[1];
    std::string output;

    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-o" && i + 1 < argc) {
            output = argv[++i];
        }
    }

    if (output.empty()) {
        output = input.substr(0, input.find_last_of('.')) + ".toru";
    } else if (output.find('/') == std::string::npos && output.find('\\') == std::string::npos) {
        // Если output без пути — создать рядом с исходником
        size_t slash = input.find_last_of("/\\");
        if (slash != std::string::npos) {
            output = input.substr(0, slash + 1) + output;
        }
    }

    tasm assembler;
    assembler.assemble_file(input);
    assembler.write_toru(output);
    std::cout << "Assembled: " << output << "\n";
    return 0;
}