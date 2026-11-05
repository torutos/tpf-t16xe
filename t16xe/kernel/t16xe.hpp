#pragma once
#include <cstdint>
#include <iostream>
#include <set>
#include <functional>

enum : uint8_t {
    HLT = 0x00,         // останов
    MOV_REG_IMM = 0x01, // MOV reg, #imm32
    MOV_REG_REG = 0x02, // MOV reg, reg
    ADD_REG_REG = 0x03, // ADD reg, reg
    SUB_REG_REG = 0x04, // SUB reg, reg
    CMP_REG_REG = 0x05, // CMP reg, reg
    JEQ = 0x06,         // JEQ смещение
    JNE = 0x07,         // JNE смещение
    JMP = 0x08,         // JMP seg:offset
    PUSH_REG = 0x09,    // PUSH reg
    POP_REG = 0x0A,     // POP reg
    MOV_REG_MEM = 0x0B, // MOV reg, [addr]  (память в регистр)
    MOV_MEM_REG = 0x0C, // MOV [addr], reg  (регистр в память)
    INC_REG = 0x0D,
    DEC_REG = 0x0E,
    MOV_REG_MEMREG = 0x0F,
    AND_REG_REG = 0x10,
    OR_REG_REG = 0x11,
    XOR_REG_REG = 0x12,
    SHL_REG_REG = 0x13,
    SHR_REG_REG = 0x14,
    CALL = 0x15,
    RET = 0x16,
    ADD_REG_IMM = 0x17,
    SUB_REG_IMM = 0x18,
    CMP_REG_IMM = 0x19,
    AND_REG_IMM = 0x1A,
    OR_REG_IMM = 0x1B,
    XOR_REG_IMM = 0x1C,
    LDB_REG_MEMREG = 0x1D, // загрузить байт из [reg] в регистр (старший байт = 0)
    STB_MEMREG_REG = 0x1E, // сохранить младший байт регистра в [reg]
    SOFTWARE_INT = 0x1F,
    IRET = 0x20,
    STI = 0x21,
    CLI = 0x22,
    LDB_REG_MEM = 0x23, // загрузить байт из [addr]
    STB_MEM_REG = 0x24, // сохранить байт в [addr],]
    MOV_MEMREG_REG = 0x25,
    MUL_REG_REG = 0x26,  // MUL reg, reg → DX:AX = AX * reg
    DIV_REG_REG = 0x27,  // DIV reg → AX = DX:AX / reg, DX = остаток
    JCS = 0x28,
    JCC = 0x29,
    NOT_REG = 0x2A,      // NOT reg
    NEG_REG = 0x2B,      // NEG reg
    PUSHF = 0x2C,        // PUSHF
    POPF = 0x2D,         // POPF
    ADC_REG_REG = 0x2E,  // ADC reg, reg (сложение с переносом)
    SBB_REG_REG = 0x2F,  // SBB reg, reg (вычитание с заёмом)
    TEST_REG_REG = 0x30, // TEST reg, reg (AND без сохранения)
    JS = 0x31,           // JS смещение (прыжок если N=1)
    JNS = 0x32,          // JNS смещение (прыжок если N=0)
    NOP = 0x33,          // NOP (нет операции)
    MOV_REG_SEG = 0x34,  // MOV reg, seg
    MOV_SEG_REG = 0x35,  // MOV seg, reg
};

enum : uint8_t {
    REG_AX = 0x00,
    REG_BX = 0x01,
    REG_CX = 0x02,
    REG_DX = 0x03,
    REG_SP = 0x04,
};

struct t16xe {
    static constexpr size_t MEMORY_SIZE = 32 * 1024 * 1024; // 32 МБ
    std::vector<uint8_t> memory;

    uint16_t AX = 0, BX = 0, CX = 0, DX = 0;
    uint16_t PC = 0;

    uint16_t SP = 0xFEEF;  // стек (пока оставим в нижней памяти, потом перенесём)
    uint16_t CS = 0x0001, DS = 0x0001, SS = 0x0001;

    bool flagC = false; // Carry
    bool flagZ = false; // Zero
    bool flagN = false; // Negative
    bool flagO = false; // Overflow
    bool flagI = true;  // Interrupt
    std::set<uint16_t> breakpoints;
    bool breakpoint_hit = false;

    static constexpr uint32_t ORG_MIN = 0x00010000;  // начало ОЗУ
    static constexpr uint32_t RAM_START = 0x00010000;    // начало ОЗУ
    static constexpr uint32_t RAM_END = 0x01FFEFFF;      // конец ОЗУ
    static constexpr uint32_t STACK_TOP = 0x01FFEFFF;    // вершина стека

    t16xe() : memory(MEMORY_SIZE) {
        error_callback = [](const std::string& msg) {
            std::cerr << "Fatal CPU error: " << msg << "\n";
            std::abort();
        };
    }
    std::function<void(char)> output_callback;
    std::function<int()> input_callback;
    std::function<void(const std::string&)> error_callback;
    void reset();
    void run();
    [[nodiscard]] static uint32_t phys_addr(uint16_t seg, uint16_t offset);
    bool execute(uint8_t opcode);
    [[nodiscard]] bool has_breakpoint(const uint16_t addr) const { return breakpoints.count(addr); }
    bool step();
    void cont();
private:
    uint16_t decode_reg();
    uint16_t decode_imm16();
    uint32_t decode_imm32();
    int8_t decode_imm8();
    void write_reg(uint16_t reg, uint16_t val);
    [[nodiscard]] uint16_t read_reg(uint16_t reg) const;
    void update_flags(uint16_t val);
    void push16(uint16_t val);
    uint16_t pop16();
    uint16_t pack_flags() const;
    void unpack_flags(uint16_t f);

};