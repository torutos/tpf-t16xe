#pragma once
#include <string>
#include <vector>

class TasmValidator {
public:
    int error_count = 0;

    void error(const std::string& msg, int line_num);
    bool validate(const std::vector<std::string>& lines);
    bool validate_file(const std::string& path);
    bool is_valid_mnemonic(const std::string& m);
private:
    bool check_operands(const std::string& mnemonic, int expected, int actual, int line_num);
};