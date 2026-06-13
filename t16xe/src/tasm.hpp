#pragma once
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <cstdint>

struct Macro {
    std::string name;
    int num_params;
    std::vector<std::string> body;
};

class tasm {
    std::map<std::string, uint8_t> opcodes = {
        {"HLT", 0x00},  {"MOV", 0x01}, // MOV reg, imm
        {"MOVR", 0x02},                // MOV reg, reg
        {"ADD", 0x03},  {"SUB", 0x04}, {"CMP", 0x05},
        {"JEQ", 0x06},  {"JNE", 0x07}, {"JMP", 0x08},
        {"PUSH", 0x09}, {"POP", 0x0A}, {"LOAD", 0x0B}, // MOV reg, [addr]
        {"STOR", 0x0C},                                // MOV [addr], reg
    };

    std::map<std::string, uint8_t> regs = {
        {"AX", 0x00}, {"BX", 0x01}, {"CX", 0x02}, {"DX", 0x03}, {"SP", 0x04},
    };

    enum class Section { None, Data, Code };
    std::map<std::string, Macro> macros;
    std::map<std::string, std::string> equ_constants;
    std::map<std::string, uint16_t> labels;
    std::vector<uint8_t> data_output;
    std::vector<uint8_t> code_output;
    uint16_t org_addr = 0x0400;
    std::string current_global_label;
public:
    void assemble_line(const std::string& line);
    void assemble_file(const std::string& path);
    void write_toru(const std::string& path);
    int instruction_size(const std::string& line);
    void collect_macros(std::vector<std::string>& lines);
    std::vector<std::string> expand_macro(const std::string& line);
    void collect_equ(std::vector<std::string>& lines);
    std::string replace_equ(const std::string& line);
    void expand_times(std::vector<std::string>& lines);
    int data_size(const std::string& line);
    void assemble_data(const std::string& line);
};