#include "tasm.hpp"
#include <exception>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <set>
#include <sstream>

int safe_stoi(const std::string& str, int base = 0)
{
    try
    {
        return std::stoi(str, nullptr, base);
    }
    catch (std::exception& e)
    {
        std::cerr << "STOI ERROR: '" << str << "' - " << e.what() << "\n";
        throw;
    }
}

std::string clean_token(const std::string& s)
{
    std::string t = s;
    // убрать запятую где угодно (только одну)
    if (const size_t comma = t.find(','); comma != std::string::npos)
        t.erase(comma, 1);
    // убрать пробелы по краям
    const size_t start = t.find_first_not_of(" \t");
    if (start == std::string::npos)
        return "";
    const size_t end = t.find_last_not_of(" \t");
    return t.substr(start, end - start + 1);
}

void tasm::assemble_line(const std::string &line)
{
    std::istringstream iss(line);
    std::string mnemonic;
    iss >> mnemonic;

    if (mnemonic == "org")
    {
        std::string arg;
        iss >> arg;
        org_addr = safe_stoi(arg);
        return;
    }
    if (mnemonic.empty() || mnemonic[0] == ';')
        return;

    if (mnemonic == "HLT")
    {
        code_output.push_back(0x00);
    }
    else if (mnemonic == "MOV")
    {
        std::string dst, src;
        iss >> dst >> src;
        dst = clean_token(dst);
        src = clean_token(src);

        // MOV CS, reg
        if (dst == "CS") {
            code_output.push_back(0x35);
            code_output.push_back(0); // CS
            code_output.push_back(regs[src]);
        }
        // MOV DS, reg
        else if (dst == "DS") {
            code_output.push_back(0x35);
            code_output.push_back(1); // DS
            code_output.push_back(regs[src]);
        }
        // MOV SS, reg
        else if (dst == "SS") {
            code_output.push_back(0x35);
            code_output.push_back(2); // SS
            code_output.push_back(regs[src]);
        }
        // MOV reg, CS
        else if (src == "CS") {
            code_output.push_back(0x34);
            code_output.push_back(regs[dst]);
            code_output.push_back(0); // CS
        }
        // MOV reg, DS
        else if (src == "DS") {
            code_output.push_back(0x34);
            code_output.push_back(regs[dst]);
            code_output.push_back(1); // DS
        }
        // MOV reg, SS
        else if (src == "SS") {
            code_output.push_back(0x34);
            code_output.push_back(regs[dst]);
            code_output.push_back(2); // SS
        }
        // MOV reg, #imm или MOV reg, label
        else if (src[0] == '#' || src[0] == '\'')
        {
            code_output.push_back(0x01);
            code_output.push_back(regs[dst]);
            uint32_t val;
            if (src[0] == '\'')
                val = static_cast<unsigned char>(src[1]);
            else if (src[0] == '#' && src.length() > 1 && src[1] == '\'')
                val = static_cast<unsigned char>(src[2]);
            else
                val = safe_stoi(src.substr(1));
            code_output.push_back(val & 0xFF);
            code_output.push_back((val >> 8) & 0xFF);
            code_output.push_back((val >> 16) & 0xFF);
            code_output.push_back((val >> 24) & 0xFF);
        }
        // MOV reg, label
        else if (labels.count(src))
        {
            code_output.push_back(0x01);
            code_output.push_back(regs[dst]);
            uint32_t val = labels[src];
            code_output.push_back(val & 0xFF);
            code_output.push_back((val >> 8) & 0xFF);
            code_output.push_back((val >> 16) & 0xFF);
            code_output.push_back((val >> 24) & 0xFF);
        }
        // MOV reg, локальная_метка
        else if (!current_global_label.empty() && labels.count(current_global_label + src))
        {
            code_output.push_back(0x01);
            code_output.push_back(regs[dst]);
            uint32_t val = labels[current_global_label + src];
            code_output.push_back(val & 0xFF);
            code_output.push_back((val >> 8) & 0xFF);
            code_output.push_back((val >> 16) & 0xFF);
            code_output.push_back((val >> 24) & 0xFF);
        }
        // MOV reg, reg
        else if (regs.count(src))
        {
            code_output.push_back(0x02);
            code_output.push_back(regs[dst]);
            code_output.push_back(regs[src]);
        }
    }
    else if (mnemonic == "STOR")
    {
        std::string src, dst;
        iss >> src >> dst;
        dst = clean_token(dst);
        src = clean_token(src);
        if (src[0] == '[')
        {
            src = src.substr(1);
            src.pop_back();
            if (regs.count(src))
            {
                code_output.push_back(0x25);
                code_output.push_back(regs[src]);
                code_output.push_back(regs[dst]);
            }
            else
            {
                uint32_t addr = safe_stoi(src);
                code_output.push_back(0x0C);
                code_output.push_back(addr & 0xFF);
                code_output.push_back((addr >> 8) & 0xFF);
                code_output.push_back((addr >> 16) & 0xFF);
                code_output.push_back((addr >> 24) & 0xFF);
                code_output.push_back(regs[dst]);
            }
        }
    }
    else if (mnemonic == "LOAD")
    {
        std::string dst, src;
        iss >> dst >> src;
        dst = clean_token(dst);
        src = clean_token(src);
        if (src[0] == '[')
        {
            src = src.substr(1);
            src.pop_back();
            if (src[0] == 'B' || src[0] == 'A' || src[0] == 'C' || src[0] == 'D' || src[0] == 'S')
            {
                code_output.push_back(0x0F);
                code_output.push_back(regs[dst]);
                code_output.push_back(regs[src]);
            }
            else
            {
                uint32_t addr = safe_stoi(src);
                code_output.push_back(0x0B);
                code_output.push_back(regs[dst]);
                code_output.push_back(addr & 0xFF);
                code_output.push_back((addr >> 8) & 0xFF);
                code_output.push_back((addr >> 16) & 0xFF);
                code_output.push_back((addr >> 24) & 0xFF);
            }
        }
    }
    else if (mnemonic == "ADD" || mnemonic == "SUB" || mnemonic == "CMP" ||
             mnemonic == "AND" || mnemonic == "OR" || mnemonic == "XOR")
    {
        std::string dst, src;
        iss >> dst >> src;
        dst = clean_token(dst);
        src = clean_token(src);
        uint8_t op_reg = (mnemonic == "ADD") ? 0x03 : (mnemonic == "SUB") ? 0x04 : (mnemonic == "CMP") ? 0x05 :
                         (mnemonic == "AND") ? 0x10 : (mnemonic == "OR") ? 0x11 : 0x12;
        uint8_t op_imm = (mnemonic == "ADD") ? 0x17 : (mnemonic == "SUB") ? 0x18 : (mnemonic == "CMP") ? 0x19 :
                         (mnemonic == "AND") ? 0x1A : (mnemonic == "OR") ? 0x1B : 0x1C;
        if (src[0] == '#')
        {
            code_output.push_back(op_imm);
            code_output.push_back(regs[dst]);
            uint32_t val;
            if (src.length() > 1 && src[1] == '\'')
                val = static_cast<unsigned char>(src[2]);
            else
                val = safe_stoi(src.substr(1));
            code_output.push_back(val & 0xFF);
            code_output.push_back((val >> 8) & 0xFF);
            code_output.push_back((val >> 16) & 0xFF);
            code_output.push_back((val >> 24) & 0xFF);
        }
        else
        {
            code_output.push_back(op_reg);
            code_output.push_back(regs[dst]);
            code_output.push_back(regs[src]);
        }
    }
    else if (mnemonic == "JEQ" || mnemonic == "JNE" || mnemonic == "JCS" || mnemonic == "JCC" ||
             mnemonic == "JS" || mnemonic == "JNS")
    {
        std::string arg;
        iss >> arg;
        int8_t offset;
        if (labels.count(arg))
            offset = static_cast<int8_t>(labels[arg] - (code_output.size() + 2));
        else if (!current_global_label.empty() && labels.count(current_global_label + arg))
            offset = static_cast<int8_t>(labels[current_global_label + arg] - (code_output.size() + 2));
        else
            offset = static_cast<int8_t>(safe_stoi(arg));
        uint8_t op = (mnemonic == "JEQ") ? 0x06 : (mnemonic == "JNE") ? 0x07 : (mnemonic == "JCS") ? 0x28 :
                     (mnemonic == "JCC") ? 0x29 : (mnemonic == "JS") ? 0x31 : 0x32;
        code_output.push_back(op);
        code_output.push_back(offset);
    }
    else if (mnemonic == "JMP")
    {
        std::string arg;
        iss >> arg;
        uint32_t addr;
        if (labels.count(arg))
            addr = labels[arg];
        else if (!current_global_label.empty() && labels.count(current_global_label + arg))
            addr = labels[current_global_label + arg];
        else
            addr = safe_stoi(arg);
        code_output.push_back(0x08);
        code_output.push_back(addr & 0xFF);
        code_output.push_back((addr >> 8) & 0xFF);
        code_output.push_back((addr >> 16) & 0xFF);
        code_output.push_back((addr >> 24) & 0xFF);
    }
    else if (mnemonic == "PUSH")
    {
        std::string reg;
        iss >> reg;
        code_output.push_back(0x09);
        code_output.push_back(regs[reg]);
    }
    else if (mnemonic == "POP")
    {
        std::string reg;
        iss >> reg;
        code_output.push_back(0x0A);
        code_output.push_back(regs[reg]);
    }
    else if (mnemonic == "INC")
    {
        std::string reg;
        iss >> reg;
        code_output.push_back(0x0D);
        code_output.push_back(regs[reg]);
    }
    else if (mnemonic == "DEC")
    {
        std::string reg;
        iss >> reg;
        code_output.push_back(0x0E);
        code_output.push_back(regs[reg]);
    }
    else if (mnemonic == "SHL" || mnemonic == "SHR")
    {
        std::string dst, src;
        iss >> dst >> src;
        dst = clean_token(dst);
        src = clean_token(src);
        code_output.push_back((mnemonic == "SHL") ? 0x13 : 0x14);
        code_output.push_back(regs[dst]);
        code_output.push_back(regs[src]);
    }
    else if (mnemonic == "CALL")
    {
        std::string arg;
        iss >> arg;
        uint32_t addr;
        if (labels.count(arg))
            addr = labels[arg];
        else if (!current_global_label.empty() && labels.count(current_global_label + arg))
            addr = labels[current_global_label + arg];
        else
            addr = safe_stoi(arg);
        code_output.push_back(0x15);
        code_output.push_back(addr & 0xFF);
        code_output.push_back((addr >> 8) & 0xFF);
        code_output.push_back((addr >> 16) & 0xFF);
        code_output.push_back((addr >> 24) & 0xFF);
    }
    else if (mnemonic == "RET")
    {
        code_output.push_back(0x16);
    }
    else if (mnemonic == "LDB")
    {
        std::string dst, src;
        iss >> dst >> src;
        dst = clean_token(dst);
        src = clean_token(src);
        if (src[0] == '[')
        {
            src = src.substr(1);
            src.pop_back();
            if (regs.count(src))
            {
                code_output.push_back(0x1D);
                code_output.push_back(regs[dst]);
                code_output.push_back(regs[src]);
            }
            else
            {
                uint32_t addr = safe_stoi(src);
                code_output.push_back(0x23);
                code_output.push_back(regs[dst]);
                code_output.push_back(addr & 0xFF);
                code_output.push_back((addr >> 8) & 0xFF);
                code_output.push_back((addr >> 16) & 0xFF);
                code_output.push_back((addr >> 24) & 0xFF);
            }
        }
    }
    else if (mnemonic == "STB")
    {
        std::string src, dst;
        iss >> src >> dst;
        dst = clean_token(dst);
        src = clean_token(src);
        if (src[0] == '[')
        {
            src = src.substr(1);
            src.pop_back();
            if (regs.count(src))
            {
                code_output.push_back(0x1E);
                code_output.push_back(regs[src]);
                code_output.push_back(regs[dst]);
            }
            else
            {
                uint32_t addr = safe_stoi(src);
                code_output.push_back(0x24);
                code_output.push_back(addr & 0xFF);
                code_output.push_back((addr >> 8) & 0xFF);
                code_output.push_back((addr >> 16) & 0xFF);
                code_output.push_back((addr >> 24) & 0xFF);
                code_output.push_back(regs[dst]);
            }
        }
    }
    else if (mnemonic == "INT")
    {
        std::string arg;
        iss >> arg;
        code_output.push_back(0x1F);
        code_output.push_back(static_cast<uint8_t>(safe_stoi(arg)));
    }
    else if (mnemonic == "IRET") code_output.push_back(0x20);
    else if (mnemonic == "STI") code_output.push_back(0x21);
    else if (mnemonic == "CLI") code_output.push_back(0x22);
    else if (mnemonic == "MUL")
    {
        std::string dst, src;
        iss >> dst >> src;
        code_output.push_back(0x26);
        code_output.push_back(regs[clean_token(dst)]);
        code_output.push_back(regs[clean_token(src)]);
    }
    else if (mnemonic == "DIV")
    {
        std::string dst, src;
        iss >> dst >> src;
        code_output.push_back(0x27);
        code_output.push_back(regs[clean_token(dst)]);
        code_output.push_back(regs[clean_token(src)]);
    }
    else if (mnemonic == "NOT") { std::string reg; iss >> reg; code_output.push_back(0x2A); code_output.push_back(regs[reg]); }
    else if (mnemonic == "NEG") { std::string reg; iss >> reg; code_output.push_back(0x2B); code_output.push_back(regs[reg]); }
    else if (mnemonic == "PUSHF") code_output.push_back(0x2C);
    else if (mnemonic == "POPF") code_output.push_back(0x2D);
    else if (mnemonic == "ADC")
    {
        std::string dst, src;
        iss >> dst >> src;
        code_output.push_back(0x2E);
        code_output.push_back(regs[clean_token(dst)]);
        code_output.push_back(regs[clean_token(src)]);
    }
    else if (mnemonic == "SBB")
    {
        std::string dst, src;
        iss >> dst >> src;
        code_output.push_back(0x2F);
        code_output.push_back(regs[clean_token(dst)]);
        code_output.push_back(regs[clean_token(src)]);
    }
    else if (mnemonic == "TEST")
    {
        std::string dst, src;
        iss >> dst >> src;
        code_output.push_back(0x30);
        code_output.push_back(regs[clean_token(dst)]);
        code_output.push_back(regs[clean_token(src)]);
    }
    else if (mnemonic == "NOP") code_output.push_back(0x33);
}

void tasm::assemble_file(const std::string &path)
{
    std::ifstream file(path);
    std::vector<std::string> lines;
    std::string line;

    while (std::getline(file, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (size_t comment = line.find(';'); comment != std::string::npos)
            line = line.substr(0, comment);
        if (line.find_first_not_of(" \t") == std::string::npos)
            continue;

        if (line.find("%include") != std::string::npos)
        {
            size_t start = line.find('"');
            size_t end = line.rfind('"');
            if (start != std::string::npos && end != std::string::npos && end > start)
            {
                std::string inc_path = line.substr(start + 1, end - start - 1);
                std::ifstream inc_file(inc_path);
                if (!inc_file)
                {
                    std::cerr << "ERROR: cannot open " << inc_path << "\n";
                    continue;
                }
                std::string inc_line;
                while (std::getline(inc_file, inc_line))
                {
                    if (!inc_line.empty() && inc_line.back() == '\r')
                        inc_line.pop_back();
                    lines.push_back(inc_line);
                }
            }
        }
        else
        {
            lines.push_back(line);
        }
    }

    collect_macros(lines);
    collect_equ(lines);

    std::vector<std::string> expanded_lines;
    for (const auto &ln : lines)
        expand_macro(ln, expanded_lines);
    lines = std::move(expanded_lines);

    for (auto &ln : lines)
        ln = replace_equ(ln);

    expand_times(lines);

    // Валидация
    errors.clear();
    error_count = 0;
    for (size_t i = 0; i < lines.size(); i++)
        validate(lines[i], static_cast<int>(i) + 1);
    if (error_count > 0) return;
    // Читаем org
    for (const auto &ln : lines)
    {
        if (ln.find("org ") == 0)
        {
            std::istringstream iss(ln);
            std::string org_str;
            uint32_t addr;
            iss >> org_str >> addr;
            org_addr = addr;
            break;
        }
    }
    // Первый проход: считаем размеры и собираем метки
    Section current = Section::None;
    uint32_t code_size = 0;
    uint32_t data_size_total = 0;

    for (const auto &ln : lines)
    {
        if (ln == "section .data") { current = Section::Data; continue; }
        if (ln == "section .code") { current = Section::Code; continue; }
        if (ln.find("org ") == 0) continue;

        std::string work = ln;
        size_t colon = work.find(':');
        if (colon != std::string::npos && colon > 0)
        {
            std::string label = work.substr(0, colon);
            label.erase(0, label.find_first_not_of(" \t"));
            label.erase(label.find_last_not_of(" \t") + 1);
            if (!label.empty() && label[0] == '.')
                label.insert(0, current_global_label);
            else
                current_global_label = label;

            if (current == Section::Code)
                labels[label] = org_addr + code_size;
            else if (current == Section::Data)
                labels[label] = org_addr + code_size + data_size_total;

            work = work.substr(colon + 1);
            if (work.find_first_not_of(" \t") == std::string::npos)
                continue;
        }

        if (current == Section::Code)
            code_size += instruction_size(work);
        else if (current == Section::Data)
            data_size_total += data_size(work);
    }

    // Второй проход: генерируем код и данные
    data_output.clear();
    code_output.clear();
    current = Section::None;

    for (const auto &ln : lines)
    {
        if (ln == "section .data") { current = Section::Data; continue; }
        if (ln == "section .code") { current = Section::Code; continue; }
        if (ln.find("org ") == 0) continue;

        std::string work = ln;
        size_t colon = work.find(':');
        if (colon != std::string::npos && colon > 0)
        {
            std::string label = work.substr(0, colon);
            label.erase(0, label.find_first_not_of(" \t"));
            label.erase(label.find_last_not_of(" \t") + 1);
            if (!label.empty() && label[0] != '.')
                current_global_label = label;
            work = work.substr(colon + 1);
            if (work.find_first_not_of(" \t") == std::string::npos)
                continue;
        }

        if (current == Section::Data)
            assemble_data(work);
        else if (current == Section::Code)
            assemble_line(work);
    }
}

void tasm::write_toru(const std::string& path) const
{
    std::ofstream file(path, std::ios::binary);
    if (!file) throw std::runtime_error("Cannot write to " + path);

    const uint32_t dsize = data_output.size();
    const uint32_t csize = code_output.size();
    const uint32_t data_addr = org_addr + csize;
    const uint8_t header[16] = {
        (uint8_t)(dsize & 0xFF), (uint8_t)(dsize >> 8), (uint8_t)(dsize >> 16), (uint8_t)(dsize >> 24),
        (uint8_t)(csize & 0xFF), (uint8_t)(csize >> 8), (uint8_t)(csize >> 16), (uint8_t)(csize >> 24),
        (uint8_t)(org_addr & 0xFF), (uint8_t)(org_addr >> 8), (uint8_t)(org_addr >> 16), (uint8_t)(org_addr >> 24),
        (uint8_t)(data_addr & 0xFF), (uint8_t)(data_addr >> 8), (uint8_t)(data_addr >> 16), (uint8_t)(data_addr >> 24)
    };
    file.write(reinterpret_cast<const char*>(header), 16);
    file.write(reinterpret_cast<const char*>(data_output.data()), dsize);
    file.write(reinterpret_cast<const char*>(code_output.data()), csize);
}

int tasm::data_size(const std::string& line)
{
    std::istringstream iss(line);
    std::string directive;
    iss >> directive;

    if (directive == "db")
    {
        std::string rest;
        std::getline(iss, rest);
        return static_cast<int>(std::count(rest.begin(), rest.end(), ',')) + 1;
    }
    if (directive == "dw")
    {
        std::string rest;
        std::getline(iss, rest);
        return (static_cast<int>(std::count(rest.begin(), rest.end(), ',')) + 1) * 2;
    }
    if (directive == "ds")
    {
        std::string str;
        std::getline(iss, str);
        str.erase(0, str.find_first_not_of(" \t"));
        str.erase(str.find_last_not_of(" \t") + 1);
        if (str.front() == '"')
            str = str.substr(1);
        if (str.back() == '"')
            str.pop_back();
        return static_cast<int>(str.size()) + 1;
    }
    if (directive == "resb")
    {
        std::string count_str;
        iss >> count_str;
        return std::stoi(count_str, nullptr, 0);
    }
    if (directive == "resw")
    {
        std::string count_str;
        iss >> count_str;
        return std::stoi(count_str, nullptr, 0) * 2;
    }
    if (directive == "fill")
    {
        std::string count_str, value_str;
        iss >> count_str >> value_str;
        return safe_stoi(count_str);
    }
    return 0;
}

void tasm::assemble_data(const std::string& line)
{
    std::istringstream iss(line);
    std::string directive;
    iss >> directive;

    if (directive == "db")
    {
        std::string token;
        while (std::getline(iss, token, ','))
        {
            token.erase(0, token.find_first_not_of(" \t"));
            uint8_t val = std::stoi(token, nullptr, 0);
            data_output.push_back(val);
        }
    }
    else if (directive == "dw")
    {
        std::string token;
        while (std::getline(iss, token, ','))
        {
            token.erase(0, token.find_first_not_of(" \t"));
            if (token[0] == '#')
                token = token.substr(1);
            uint16_t val = std::stoi(token, nullptr, 0);
            data_output.push_back(val & 0xFF);
            data_output.push_back(val >> 8);
        }
    }
    else if (directive == "ds")
    {
        std::string str;
        std::getline(iss, str);
        str.erase(0, str.find_first_not_of(" \t"));
        str.erase(str.find_last_not_of(" \t") + 1);
        if (str.front() == '"')
            str = str.substr(1);
        if (str.back() == '"')
            str.pop_back();
        for (char c : str)
            data_output.push_back(c);
        data_output.push_back(0);
    }
    else if (directive == "resb")
    {
        std::string count_str;
        iss >> count_str;
        int count = std::stoi(count_str, nullptr, 0);
        for (int i = 0; i < count; i++)
            data_output.push_back(0);
    }
    else if (directive == "resw")
    {
        std::string count_str;
        iss >> count_str;
        int count = std::stoi(count_str, nullptr, 0);
        for (int i = 0; i < count * 2; i++)
            data_output.push_back(0);
    }
    else if (directive == "fill")
    {
        std::string count_str, value_str;
        iss >> count_str >> value_str;
        int count = safe_stoi(count_str);
        uint8_t val = safe_stoi(value_str);
        for (int i = 0; i < count; i++)
            data_output.push_back(val);
    }
}

int tasm::instruction_size(const std::string &line) const
{
    std::istringstream iss(line);
    std::string mnemonic;
    iss >> mnemonic;
    if (mnemonic.empty() || mnemonic[0] == ';') return 0;
    if (mnemonic.back() == ':') return 0;

    // 1 байт
    if (mnemonic == "HLT" || mnemonic == "RET" || mnemonic == "IRET" ||
        mnemonic == "STI" || mnemonic == "CLI" || mnemonic == "PUSHF" ||
        mnemonic == "POPF" || mnemonic == "NOP") return 1;

    // 2 байта
    if (mnemonic == "PUSH" || mnemonic == "POP" || mnemonic == "INC" ||
        mnemonic == "DEC" || mnemonic == "INT" || mnemonic == "NOT" ||
        mnemonic == "NEG") return 2;

    // Условные переходы — 2 байта
    if (mnemonic == "JEQ" || mnemonic == "JNE" || mnemonic == "JCS" ||
        mnemonic == "JCC" || mnemonic == "JS" || mnemonic == "JNS") return 2;

    // 3 байта (reg, reg)
    if (mnemonic == "MOV") {
        std::string m, dst, src;
        std::istringstream tmp(line);
        tmp >> m >> dst >> src;
        src = clean_token(src);
        dst = clean_token(dst);
        if (dst == "CS" || dst == "DS" || dst == "SS") return 3;
        if (src == "CS" || src == "DS" || src == "SS") return 3;
        if (regs.count(src)) return 3;
        return 6;
    }
    if (mnemonic == "SHL" || mnemonic == "SHR" || mnemonic == "MUL" ||
        mnemonic == "DIV" || mnemonic == "ADC" || mnemonic == "SBB" ||
        mnemonic == "TEST") return 3;

    // ADD/SUB/CMP/AND/OR/XOR — 3 или 6
    if (mnemonic == "ADD" || mnemonic == "SUB" || mnemonic == "CMP" ||
        mnemonic == "AND" || mnemonic == "OR" || mnemonic == "XOR") {
        if (line.find('#') != std::string::npos) return 6;
        return 3;
    }

    // LDB/STB — 3 или 6
    if (mnemonic == "LDB" || mnemonic == "STB") {
        size_t br = line.find('[');
        if (br != std::string::npos) {
            std::string inside = line.substr(br + 1);
            size_t cl = inside.find(']');
            if (cl != std::string::npos) inside = inside.substr(0, cl);
            inside = clean_token(inside);
            if (regs.count(inside)) return 3;
        }
        return 6;
    }

    // LOAD/STOR — 3 или 6
    if (mnemonic == "LOAD" || mnemonic == "STOR") {
        size_t br = line.find('[');
        if (br != std::string::npos) {
            std::string inside = line.substr(br + 1);
            size_t cl = inside.find(']');
            if (cl != std::string::npos) inside = inside.substr(0, cl);
            inside = clean_token(inside);
            if (regs.count(inside)) return 3;
        }
        return 6;
    }

    // JMP/CALL — 6 байт
    if (mnemonic == "JMP" || mnemonic == "CALL") return 5;

    return 0;
}
void tasm::collect_macros(std::vector<std::string>& lines)
{
    std::vector<std::string> new_lines;
    bool in_macro = false;
    Macro current_macro;

    for (const auto& ln : lines)
    {
        std::istringstream iss(ln);
        std::string first_word;
        iss >> first_word;

        if (first_word == "%macro")
        {
            in_macro = true;
            current_macro.body.clear();
            iss >> current_macro.name >> current_macro.num_params;
        }
        else if (first_word == "%endmacro")
        {
            in_macro = false;
            macros[current_macro.name] = current_macro;
        }
        else if (!in_macro)
        {
            new_lines.push_back(ln);
        }
        else
        {
            current_macro.body.push_back(ln);
        }
    }

    lines = std::move(new_lines);
}

void tasm::expand_macro(const std::string& line, std::vector<std::string>& out)
{
    std::istringstream iss(line);
    std::string name;
    iss >> name;

    if (!macros.count(name))
    {
        out.push_back(line);
        return;
    }

    Macro& m = macros[name];
    std::vector<std::string> args;
    std::string arg;
    while (iss >> arg) args.push_back(arg);

    for (const auto& body_line : m.body)
    {
        std::string expanded = body_line;
        for (size_t i = 0; i < args.size(); i++)
        {
            std::string placeholder = "%" + std::to_string(i + 1);
            size_t pos = expanded.find(placeholder);
            if (pos != std::string::npos)
            {
                expanded.replace(pos, placeholder.length(), args[i]);
            }
        }
        out.push_back(expanded);
    }
}

void tasm::collect_equ(std::vector<std::string>& lines)
{
    std::vector<std::string> new_lines;

    for (const auto& ln : lines)
    {
        std::istringstream iss(ln);
        std::string name, equ_directive;
        iss >> name >> equ_directive;

        if (equ_directive == "equ")
        {
            std::string value;
            std::getline(iss, value);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            equ_constants[name] = value;
        }
        else
        {
            new_lines.push_back(ln);
        }
    }

    lines = std::move(new_lines);
}

std::string tasm::replace_equ(const std::string& line) const
{
    std::string result = line;
    for (const auto& [fst, snd] : equ_constants)
    {
        const std::string& name = fst;
        const std::string& value = snd;
        size_t pos = 0;

        while ((pos = result.find(name, pos)) != std::string::npos)
        {
            const bool valid_start = (pos == 0 || !isalnum(result[pos - 1]));

            if (const bool valid_end = (pos + name.length() >= result.length() || !isalnum(result[pos + name.length()]))
                ; valid_start && valid_end)
            {
                result.replace(pos, name.length(), value);
                pos += value.length();
            }
            else
            {
                pos += name.length();
            }
        }
    }
    return result;
}

void tasm::expand_times(std::vector<std::string>& lines)
{
    std::vector<std::string> new_lines;

    for (const auto& ln : lines)
    {
        std::istringstream iss(ln);
        std::string first_word;
        iss >> first_word;

        if (first_word == "times")
        {
            int count;
            iss >> count;

            // получить оставшуюся часть строки (инструкцию)
            std::string rest;
            std::getline(iss, rest);
            rest.erase(0, rest.find_first_not_of(" \t"));

            for (int i = 0; i < count; i++)
            {
                new_lines.push_back(rest);
            }
        }
        else
        {
            new_lines.push_back(ln);
        }
    }

    lines = std::move(new_lines);
}

bool tasm::validate(const std::string& line, int line_num)
{
    std::istringstream iss(line);
    std::string mnemonic;
    iss >> mnemonic;
    if (mnemonic == "org")
    {
        std::string arg;
        iss >> arg;
        int addr = safe_stoi(arg);
        if (addr < 0x000400)
        {
            errors.push_back(
                "Line " + std::to_string(line_num) + ": org address too low, minimum is 0x000400 (RAM start)");
            error_count++;
            return false;
        }
    }

    if (mnemonic.empty() || mnemonic[0] == ';') return true;
    if (mnemonic.back() == ':') return true;

    // Пропускаем директивы
    if (mnemonic == "org" || mnemonic == "section" ||
        mnemonic == "%macro" || mnemonic == "%endmacro" ||
        mnemonic == "%include" || mnemonic == "%define")
        return true;

    static const std::set<std::string> valid = {
        "HLT", "MOV", "ADD", "SUB", "CMP", "AND", "OR", "XOR",
        "SHL", "SHR", "INC", "DEC", "PUSH", "POP",
        "JMP", "JEQ", "JNE", "JCS", "JCC", "CALL", "RET",
        "LDB", "STB", "STOR", "LOAD",
        "INT", "IRET", "STI", "CLI",
        "MUL", "DIV", "NOT", "NEG", "PUSHF", "POPF",
        "ADC", "SBB", "TEST", "JS", "JNS", "NOP"
    };

    if (!valid.count(mnemonic))
    {
        errors.push_back("Line " + std::to_string(line_num) + ": unknown instruction '" + mnemonic + "'");
        error_count++;
        return false;
    }

    // Считаем операнды
    int operands = 0;
    std::string token;
    while (iss >> token) operands++;

    // Проверяем количество операндов
    static const std::map<std::string, int> expected = {
        {"HLT", 0}, {"RET", 0}, {"IRET", 0}, {"STI", 0}, {"CLI", 0},
        {"PUSH", 1}, {"POP", 1}, {"INC", 1}, {"DEC", 1}, {"INT", 1},
        {"CALL", 1}, {"JMP", 1}, {"JEQ", 1}, {"JNE", 1}, {"JCS", 1}, {"JCC", 1},
        {"MOV", 2}, {"ADD", 2}, {"SUB", 2}, {"CMP", 2}, {"AND", 2}, {"OR", 2},
        {"XOR", 2}, {"SHL", 2}, {"SHR", 2}, {"STOR", 2}, {"LOAD", 2}, {"LDB", 2}, {"STB", 2},
        // Новые
        {"MUL", 2}, {"DIV", 2}, {"NOT", 1}, {"NEG", 1},
        {"PUSHF", 0}, {"POPF", 0},
        {"ADC", 2}, {"SBB", 2}, {"TEST", 2},
        {"JS", 1}, {"JNS", 1}, {"NOP", 0}
    };

    auto it = expected.find(mnemonic);
    if (it != expected.end() && operands < it->second)
    {
        errors.push_back(
            "Line " + std::to_string(line_num) + ": " + mnemonic + " expects " + std::to_string(it->second) +
            " operands, got " + std::to_string(operands));
        error_count++;
        return false;
    }

    return true;
}
