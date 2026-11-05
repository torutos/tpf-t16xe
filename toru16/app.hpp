// app.hpp
#pragma once
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <algorithm>
#include "t16xe.hpp"
#include "tools.hpp"

struct DiskFile {
    std::string name;
    uint32_t size;
    uint32_t offset;
    bool bootable;
};

struct Toru16App {
    t16xe cpu;
    std::string terminalOutput;
    bool running = false;
    bool paused = false;
    bool outputUpdated = false;
    bool diskLoaded = false;
    std::string diskPath;
    std::vector<DiskFile> files;
    uint32_t diskSize = 0;

    Toru16App() {
        cpu.output_callback = [this](char c) {
            terminalOutput += c;
            outputUpdated = true;
        };
        cpu.error_callback = [this](const std::string& msg) {
            terminalOutput += "CPU Error: " + msg + "\n";
            running = false; // останавливаем выполнение, но не крашим GUI
        };
        std::ifstream biosFile("bios.toru", std::ios::binary);
        if (biosFile) {
            uint32_t dsize = biosFile.get() | (biosFile.get() << 8) | (biosFile.get() << 16) | (biosFile.get() << 24);
            uint32_t csize = biosFile.get() | (biosFile.get() << 8) | (biosFile.get() << 16) | (biosFile.get() << 24);
            uint32_t code_start = biosFile.get() | (biosFile.get() << 8) | (biosFile.get() << 16) | (biosFile.get() << 24);
            uint32_t data_start = biosFile.get() | (biosFile.get() << 8) | (biosFile.get() << 16) | (biosFile.get() << 24);
            biosFile.read(reinterpret_cast<char*>(&cpu.memory[data_start]), dsize);
            biosFile.read(reinterpret_cast<char*>(&cpu.memory[code_start]), csize);
            cpu.memory[0x1FFFFF4] = code_start & 0xFF;
            cpu.memory[0x1FFFFF5] = (code_start >> 8) & 0xFF;
            cpu.memory[0x1FFFFF6] = (code_start >> 16) & 0xFF;
            cpu.memory[0x1FFFFF7] = (code_start >> 24) & 0xFF;
        }
    }

    void start() {
        terminalOutput.clear();
        cpu.reset();
        running = true;
        paused = false;
    }

    void bootFromDisk() {
        cpu.reset();
        running = true;
        paused = false;
    }

    void pause() { paused = !paused; }
    void powerDown() { running = false; terminalOutput.clear(); }
    void loadProgram(const std::string& path) { load_toru(cpu, path); }

    void loadDisk(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) { terminalOutput += "Cannot open disk: " + path + "\n"; return; }
        file.seekg(0, std::ios::end); diskSize = file.tellg(); file.seekg(0);
        file.seekg(510);
        if ((file.get() | (file.get() << 8)) != 0xAA55) { terminalOutput += "Invalid boot sector\n"; return; }
        file.seekg(512);
        char magic[4]; file.read(magic, 4);
        if (std::string(magic, 4) != "TORU") { terminalOutput += "Invalid disk\n"; return; }
        uint16_t version = file.get() | (file.get() << 8);
        files.clear();
        file.seekg(1024);
        for (int i = 0; i < 1024; i++) {
            char name[16] = {}; file.read(name, 16);
            if (name[0] == 0) break;
            DiskFile df;
            df.name = name;
            df.size = file.get() | (file.get() << 8) | (file.get() << 16) | (file.get() << 24);
            df.offset = file.get() | (file.get() << 8) | (file.get() << 16) | (file.get() << 24);
            uint32_t attrs = file.get() | (file.get() << 8) | (file.get() << 16) | (file.get() << 24);
            df.bootable = (attrs & 1) != 0;
            file.seekg(4, std::ios::cur);
            files.push_back(df);
        }
        terminalOutput += "Disk: " + path + " v" + std::to_string(version) + ", " + std::to_string(files.size()) + " files\n";
        diskLoaded = true; diskPath = path;
    }

    void createDisk(const std::string& path) {
        std::ofstream file(path, std::ios::binary);
        if (!file) return;
        std::vector<uint8_t> boot(512, 0); boot[510] = 0x55; boot[511] = 0xAA;
        file.write(reinterpret_cast<char*>(boot.data()), 512);
        uint8_t super[512] = {}; super[0]='T'; super[1]='O'; super[2]='R'; super[3]='U'; super[4]=0x01;
        file.write(reinterpret_cast<char*>(super), 512);
        std::vector<uint8_t> table(32*1024, 0);
        file.write(reinterpret_cast<char*>(table.data()), 32*1024);
        terminalOutput += "Created: " + path + "\n";
        diskLoaded = true; diskPath = path; files.clear();
    }

    void loadFromDisk(const std::string& name) {
        if (!diskLoaded) { terminalOutput += "No disk\n"; return; }
        for (auto& f : files) {
            if (f.name == name) {
                std::ifstream file(diskPath, std::ios::binary);
                file.seekg(f.offset);
                std::vector<uint8_t> data(f.size);
                file.read(reinterpret_cast<char*>(data.data()), f.size);

                uint32_t dsize = data[0] | (data[1]<<8) | (data[2]<<16) | (data[3]<<24);
                uint32_t csize = data[4] | (data[5]<<8) | (data[6]<<16) | (data[7]<<24);
                uint32_t code_start = data[8] | (data[9]<<8) | (data[10]<<16) | (data[11]<<24);
                uint32_t data_start = data[12] | (data[13]<<8) | (data[14]<<16) | (data[15]<<24);

                for (uint32_t i = 0; i < dsize; i++)
                    cpu.memory[data_start + i] = data[16 + i];
                for (uint32_t i = 0; i < csize; i++)
                    cpu.memory[code_start + i] = data[16 + dsize + i];

                cpu.memory[0x1FFFFF4] = code_start & 0xFF;
                cpu.memory[0x1FFFFF5] = (code_start >> 8) & 0xFF;
                cpu.memory[0x1FFFFF6] = (code_start >> 16) & 0xFF;
                cpu.memory[0x1FFFFF7] = (code_start >> 24) & 0xFF;

                terminalOutput += "Loaded " + name + " (code: " + std::to_string(csize) + " bytes at 0x" + std::to_string(code_start) + ")\n";
                return;
            }
        }
        terminalOutput += "Not found: " + name + "\n";
    }
// TODO: delete writeFileToDisk
    void writeFileToDisk(const std::string& hostPath) {
        if (!diskLoaded) { terminalOutput += "No disk\n"; return; }
        std::ifstream src(hostPath, std::ios::binary);
        if (!src) { terminalOutput += "Cannot open: " + hostPath + "\n"; return; }
        src.seekg(0, std::ios::end); uint32_t size = src.tellg(); src.seekg(0);
        std::vector<uint8_t> data(size);
        src.read(reinterpret_cast<char*>(data.data()), size);

        uint32_t dataStart = 512+512+32768, offset = dataStart;
        for (auto& f : files) offset = std::max(offset, f.offset + f.size);

        std::string name = hostPath;
        size_t s = name.find_last_of("/\\");
        if (s != std::string::npos) name = name.substr(s+1);
        bool bootable = (name == "boot.toru");

        std::fstream disk(diskPath, std::ios::binary|std::ios::in|std::ios::out);
        disk.seekp(offset); disk.write(reinterpret_cast<char*>(data.data()), size);

        uint32_t t = 1024 + files.size()*32; disk.seekp(t);
        char n[16]={}; strncpy(n,name.c_str(),15); disk.write(n,16);
        disk.put(size&0xFF); disk.put((size>>8)&0xFF); disk.put((size>>16)&0xFF); disk.put((size>>24)&0xFF);
        disk.put(offset&0xFF); disk.put((offset>>8)&0xFF); disk.put((offset>>16)&0xFF); disk.put((offset>>24)&0xFF);
        disk.put(bootable?1:0); disk.put(0); disk.put(0); disk.put(0);
        disk.put(0); disk.put(0); disk.put(0); disk.put(0);

        files.push_back({name, size, offset, bootable});
        terminalOutput += "Written: " + name + " (" + std::to_string(size) + " bytes)";
        if(bootable) terminalOutput += " [bootable]";
        terminalOutput += "\n";
    }

    void executeCommand(const std::string& cmd) {
        terminalOutput += "> " + cmd + "\n"; outputUpdated = true;
        std::istringstream iss(cmd);
        std::string c; iss >> c;

        if (c == "help") terminalOutput += "help, clear, info, disk, dir, write, run, boot\n";
        else if (c == "clear") terminalOutput.clear();
        else if (c == "info") {
            terminalOutput += "Toru16 v0.1a\n32MB RAM\n";
            if (diskLoaded) terminalOutput += "Disk: " + diskPath + " (" + std::to_string(diskSize) + " bytes)\nFiles: " + std::to_string(files.size()) + "\n";
        }
        else if (c == "disk") {
            if (!diskLoaded) createDisk("c.tdsk");
            else terminalOutput += "Disk: " + diskPath + "\n";
        }
        else if (c == "dir") {
            if (!diskLoaded) terminalOutput += "No disk\n";
            else if (files.empty()) terminalOutput += "No files\n";
            else for (auto& f : files) { terminalOutput += f.name + " (" + std::to_string(f.size) + ")"; if(f.bootable) terminalOutput += " [boot]"; terminalOutput += "\n"; }
        }
        else if (c == "write") {
            std::string f; iss >> f;
            if(!f.empty()) writeFileToDisk(f);
            else terminalOutput += "Usage: write <file>\n";
        }
        else if (c == "run") {
            std::string f; iss >> f;
            if(!f.empty()) {
                loadFromDisk(f);
                bootFromDisk();
                terminalOutput += "Running " + f + "\n";
            } else terminalOutput += "Usage: run <file>\n";
        }
        else if (c == "boot") {
            for (auto& ff : files) {
                if (ff.bootable) {
                    loadFromDisk(ff.name);
                    bootFromDisk();
                    terminalOutput += "Booting from " + ff.name + "\n";
                    outputUpdated = true;
                    return;
                }
            }
            terminalOutput += "No bootable file. Write 'boot.toru' to disk.\n";
        }
        else if (!c.empty()) terminalOutput += "Unknown: " + c + "\n";
        outputUpdated = true;
    }
};