#pragma once
#include <cstdint>
#include <iostream>
#include <memory>
using namespace std;

enum : uint8_t {
    HLT = 0x00,         // останов
    MOV_REG_IMM = 0x01, // MOV reg, #imm16
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
    INT = 0x1F,
    IRET = 0x20,
    STI = 0x21,
    CLI = 0x22,
    LDB_REG_MEM = 0x23, // загрузить байт из [addr]
    STB_MEM_REG = 0x24, // сохранить байт в [addr],]
    MOV_MEMREG_REG = 0x25,
    JCS = 0x28,
    JCC = 0x29,
};

enum : uint8_t {
    REG_AX = 0x00,
    REG_BX = 0x01,
    REG_CX = 0x02,
    REG_DX = 0x03,
    REG_SP = 0x04,
};

struct t16xe {
    static constexpr size_t MEMORY_SIZE = 1048576; // 1 МБ
    unique_ptr<uint8_t[]> memory;

    uint16_t AX = 0, BX = 0, CX = 0, DX = 0;
    uint16_t PC = 0;
    uint16_t SP = 0xFFFE;

    uint8_t CS = 0, DS = 0x01, SS = 0;

    bool flagC = false; // Carry
    bool flagZ = false; // Zero
    bool flagN = false; // Negative
    bool flagO = false; // Overflow
    bool flagI = true;  // Interrupt Disable — после сброса прерывания запрещены

    t16xe() { memory = make_unique<uint8_t[]>(MEMORY_SIZE); }

    inline uint32_t phys_addr(uint8_t seg, uint16_t offset) const {
        if (offset >= 0xFFF0) {
            return offset; // порты
        }

        return ((uint32_t)seg << 16) | offset;
    }

    void reset();
    void run();

private:
    bool execute(uint8_t opcode);
    uint8_t decode_reg8();
    uint16_t decode_reg();
    uint16_t decode_imm16();
    int8_t decode_imm8();
    void write_reg(uint16_t reg, uint16_t val);
    uint16_t read_reg(uint16_t reg) const;
    void update_flags(uint16_t val);
};