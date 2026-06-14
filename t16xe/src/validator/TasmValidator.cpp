#include "TasmValidator.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>

void TasmValidator::error(const std::string &msg, int line_num)
{
    std::cerr << "Error line " << line_num << ": " << msg << "\n";
    error_count++;
}

bool TasmValidator::check_operands(const std::string &mnemonic, int expected, int actual, int line_num)
{
    if (actual < expected)
    {
        error(mnemonic + ": expected " + std::to_string(expected) + " operands, got " + std::to_string(actual), line_num);
        return false;
    }
    return true;
}

bool TasmValidator::is_valid_mnemonic(const std::string &m)
{
    static const std::vector<std::string> valid = {
        "HLT", "MOV", "ADD", "SUB", "CMP", "AND", "OR", "XOR",
        "SHL", "SHR", "INC", "DEC", "PUSH", "POP",
        "JMP", "JEQ", "JNE", "JCS", "JCC", "CALL", "RET",
        "LDB", "STB", "STOR", "LOAD",
        "INT", "IRET", "STI", "CLI"};
    return std::find(valid.begin(), valid.end(), m) != valid.end();
}

bool TasmValidator::validate(const std::vector<std::string> &lines)
{
    error_count = 0;

    for (size_t i = 0; i < lines.size(); i++)
    {
        std::string line = lines[i];
        int line_num = i + 1;

        // Пропускаем пустые строки, комментарии, директивы
        if (line.empty() || line[0] == ';')
            continue;
        if (line[0] == '.')
            continue; // метки

        std::istringstream iss(line);
        std::string mnemonic;
        if (!(iss >> mnemonic))
            continue;
        

        // Пропускаем директивы
        if (mnemonic == "org" || mnemonic == "section" ||
            mnemonic == "%macro" || mnemonic == "%endmacro" ||
            mnemonic == "%include")
            continue;
        if (!mnemonic.empty() && mnemonic.back() == ':') continue;
        if (!is_valid_mnemonic(mnemonic))
        {
            error("Unknown instruction: " + mnemonic, line_num);
            continue;
        }
        // Считаем операнды (слова после мнемоники)
        int operands = 0;
        std::string token;
        while (iss >> token)
            operands++;

        // Проверяем количество операндов
        if (mnemonic == "HLT" || mnemonic == "RET" || mnemonic == "IRET" ||
            mnemonic == "STI" || mnemonic == "CLI")
        {
            check_operands(mnemonic, 0, operands, line_num);
        }
        else if (mnemonic == "PUSH" || mnemonic == "POP" || mnemonic == "INC" ||
                 mnemonic == "DEC" || mnemonic == "INT")
        {
            check_operands(mnemonic, 1, operands, line_num);
        }
        else if (mnemonic == "MOV" || mnemonic == "ADD" || mnemonic == "SUB" ||
                 mnemonic == "CMP" || mnemonic == "AND" || mnemonic == "OR" ||
                 mnemonic == "XOR" || mnemonic == "SHL" || mnemonic == "SHR" ||
                 mnemonic == "STOR" || mnemonic == "LOAD" || mnemonic == "LDB" ||
                 mnemonic == "STB")
        {
            check_operands(mnemonic, 2, operands, line_num);
        }
        else if (mnemonic == "CALL" || mnemonic == "JMP" ||
                 mnemonic == "JEQ" || mnemonic == "JNE" ||
                 mnemonic == "JCS" || mnemonic == "JCC")
        {
            check_operands(mnemonic, 1, operands, line_num);
        }
    }

    if (error_count > 0)
    {
        std::cerr << "Validation failed with " << error_count << " errors.\n";
        return false;
    }
    return true;
}
bool TasmValidator::validate_file(const std::string &path)
{
    std::ifstream file(path);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        lines.push_back(line);
    }
    return validate(lines);
}