// tcc.cpp — Toru C Compiler (простой транслятор)
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cctype>
#include <algorithm>
#include "../t16xe/src/asm/tasm.hpp"
#include "../t16xe/src/kernel/t16xe.hpp"
#include "../t16xe/src/kernel/tools.hpp"

class TCC {
    std::vector<std::string> output;
    int label_counter = 0;
    
public:
    void emit(const std::string& line) {
        output.push_back("    " + line);
    }
    
    std::string new_label() {
        return ".L" + std::to_string(++label_counter);
    }
    
    // Компиляция выражения, результат в AX
    void expr(const std::string& s) {
        // Убираем пробелы
        std::string e = s;
        e.erase(std::remove_if(e.begin(), e.end(), [](unsigned char c) { return std::isspace(c); }), e.end());
        
        // Если это просто число
        bool is_num = true;
        for (char c : e) if (!isdigit(c)) { is_num = false; break; }
        if (is_num) {
            emit("MOV AX, #" + e);
            return;
        }
        
        // Сложение: a+b
        size_t plus = e.find('+');
        if (plus != std::string::npos) {
            std::string left = e.substr(0, plus);
            std::string right = e.substr(plus + 1);
            expr(right);
            emit("PUSH AX");
            expr(left);
            emit("POP BX");
            emit("ADD AX, BX");
            return;
        }
        
        // Вычитание: a-b
        size_t minus = e.find('-');
        if (minus != std::string::npos) {
            std::string left = e.substr(0, minus);
            std::string right = e.substr(minus + 1);
            expr(right);
            emit("PUSH AX");
            expr(left);
            emit("POP BX");
            emit("SUB AX, BX");
            return;
        }
        
        // Умножение: a*b
        size_t star = e.find('*');
        if (star != std::string::npos) {
            std::string left = e.substr(0, star);
            std::string right = e.substr(star + 1);
            expr(right);
            emit("PUSH AX");
            expr(left);
            emit("POP BX");
            emit("CALL mul");
            return;
        }
        
        // Идентификатор (переменная)
        emit("MOV AX, " + e);
    }
    
    void write(const std::string& path) {
        std::ofstream file(path);
        file << "section .code\n";
        file << "    org 0x0400\n";
        file << "    CALL main\n";
        file << "    HLT\n\n";
        
        // Библиотека
        file << "mul:\n";
        file << "    PUSH CX\n";
        file << "    MOV CX, AX\n";
        file << "    MOV AX, #0\n";
        file << "mul_loop:\n";
        file << "    CMP BX, #0\n";
        file << "    JEQ mul_done\n";
        file << "    ADD AX, CX\n";
        file << "    DEC BX\n";
        file << "    JMP mul_loop\n";
        file << "mul_done:\n";
        file << "    POP CX\n";
        file << "    RET\n\n";
        
        file << "print_int:\n";
        file << "    PUSH BX\n";
        file << "    PUSH CX\n";
        file << "    PUSH DX\n";
        file << "    MOV BX, #100\n";
        file << "    CALL print_digit\n";
        file << "    MOV BX, #10\n";
        file << "    CALL print_digit\n";
        file << "    MOV BX, #1\n";
        file << "    CALL print_digit\n";
        file << "    POP DX\n";
        file << "    POP CX\n";
        file << "    POP BX\n";
        file << "    RET\n\n";
        
        file << "print_digit:\n";
        file << "    MOV CX, #0\n";
        file << "pd_loop:\n";
        file << "    CMP AX, BX\n";
        file << "    JCC pd_done\n";
        file << "    SUB AX, BX\n";
        file << "    INC CX\n";
        file << "    JMP pd_loop\n";
        file << "pd_done:\n";
        file << "    PUSH AX\n";
        file << "    MOV AX, CX\n";
        file << "    ADD AX, #48\n";
        file << "    STB [0xFFF2], AX\n";
        file << "    POP AX\n";
        file << "    RET\n\n";
        
        // Код программы
        for (auto& line : output) {
            file << line << "\n";
        }
        file << "\n";
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: tcc <file.tc> [-o <output.toru>] [-run]\n";
        return 1;
    }

    std::string input = argv[1];
    std::string asm_file = input.substr(0, input.find_last_of('.')) + ".tsm";
    std::string toru_file = input.substr(0, input.find_last_of('.')) + ".toru";
    bool run = false;

    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-run") run = true;
        else if (arg == "-o" && i + 1 < argc) toru_file = argv[++i];
    }

    // Читаем .tc
    std::ifstream file(input);
    if (!file) {
        std::cerr << "Cannot open " << input << "\n";
        return 1;
    }
    std::string source((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

    // Шаг 1: C → ассемблер
    TCC compiler;
    // ... парсинг и генерация ...
    compiler.write(asm_file);
    std::cout << "Compiled to " << asm_file << "\n";

    // Шаг 2: Ассемблер → .toru
    tasm assembler;
    assembler.assemble_file(asm_file);
    assembler.write_toru(toru_file);
    std::cout << "Assembled to " << toru_file << "\n";

    // Шаг 3: Запуск
    if (run) {
        std::cout << "Running...\n";
        t16xe cpu;
        load_toru(cpu, toru_file);
        cpu.reset();
        cpu.run();
    }

    return 0;
}