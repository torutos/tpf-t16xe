#pragma once
#include <string>
#include "kernel/t16xe.hpp"
#include "asm/tasm.hpp"
#include "TextEditor.h"
#include "kernel/tools.hpp"

struct AppState {
    t16xe cpu;
    bool loaded = false;
    bool halted = false;

    TextEditor editor;
    std::string currentFile = "program.tsm";
    std::string terminalOutput;
    bool terminalUpdated = false;
    std::string errorMessages;
    void loadProgram(const std::string& path) {
        tasm assembler;
        assembler.assemble_file(path);

        errorMessages.clear();
        for (auto& e : assembler.errors) {
            errorMessages += e + "\n";
        }

        if (assembler.error_count > 0) {
            loaded = false;
            return;
        }
        std::string toru = path.substr(0, path.find_last_of('.')) + ".toru";
        assembler.write_toru(toru);

        load_toru(cpu, toru);
        cpu.reset();
        loaded = true;
        halted = false;
        terminalOutput.clear();
        cpu.breakpoint_hit = false;

        cpu.output_callback = [this](char c) {
            terminalOutput += c;
            terminalUpdated = true;
        };
    }
};
