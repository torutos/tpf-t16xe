#pragma once
#include <string>
#include "t16xe.hpp"
#include "tools.hpp"

struct Toru16App {
    t16xe cpu;
    std::string terminalOutput;
    bool running = false;
    bool paused = false;
    bool outputUpdated = false;

    Toru16App() {
        cpu.output_callback = [this](char c) {
            terminalOutput += c;
            outputUpdated = true;
        };
    }

    void start() {
        terminalOutput.clear();
        cpu.reset();
        running = true;
        paused = false;
    }

    void pause() { paused = !paused; }
    void powerDown() { running = false; terminalOutput.clear(); }

    void loadProgram(const std::string& path) {
        load_toru(cpu, path);
    }

    void loadDisk(const std::string& path) {
        // TODO
    }

    void executeCommand(const std::string& cmd) {
        terminalOutput += "> " + cmd + "\n";
        outputUpdated = true;

        if (cmd == "help") {
            terminalOutput += "Commands: help, clear, info, run, stop\n";
        } else if (cmd == "clear") {
            terminalOutput.clear();
        } else if (cmd == "info") {
            terminalOutput += "Toru16 v0.1a\n32MB RAM\nPorts: 0x1FFFFF0-0x1FFFFF2\n";
        } else if (cmd == "run") {
            start();
        } else if (cmd == "stop") {
            powerDown();
        } else {
            terminalOutput += "Unknown command: " + cmd + "\n";
        }
        outputUpdated = true;
    }
};