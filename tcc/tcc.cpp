// tcc.cpp — Toru C Compiler (простой транслятор)
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cctype>
#include <algorithm>

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
        e.erase(std::remove_if(e.begin(), e.end(), isspace), e.end());
        
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
        file << "    ; AX = AX * BX (простое умножение сложением)\n";
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
        file << "    ; вывод числа из AX (0-999)\n";
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
        file << "    ; выводит (AX / BX) % 10 как цифру\n";
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
        std::cerr << "Usage: tcc <file.c>\n";
        std::cerr << "  Supports: print_int(expr);\n";
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Cannot open " << argv[1] << "\n";
        return 1;
    }

    TCC compiler;
    std::string line;
    bool in_main = false;
    
    while (std::getline(file, line)) {
        // Убираем пробелы по краям
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        size_t end = line.find_last_not_of(" \t\r\n");
        line = line.substr(start, end - start + 1);
        
        // Пропускаем всё, что не в main
        if (line.find("int main") != std::string::npos) {
            in_main = true;
            compiler.emit("main:");
            continue;
        }
        if (line == "{") continue;
        if (line == "}") {
            compiler.emit("RET");
            break;
        }
        if (!in_main) continue;
        
        // print_int(expr);
        if (line.find("print_int(") == 0) {
            std::string arg = line.substr(10, line.length() - 12); // print_int( ... );
            compiler.expr(arg);
            compiler.emit("CALL print_int");
            continue;
        }
        
        // return expr;
        if (line.find("return ") == 0) {
            std::string arg = line.substr(7, line.length() - 8); // return ... ;
            compiler.expr(arg);
            continue;
        }
    }

    std::string output = std::string(argv[1]).substr(0, std::string(argv[1]).find_last_of('.')) + ".asm";
    compiler.write(output);
    std::cout << "Compiled to " << output << "\n";
    return 0;
}