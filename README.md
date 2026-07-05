# T16XE — 16-bit CPU Emulator & Toolchain

A fully custom 16-bit processor emulator written in C++17, featuring its own instruction set, assembler, binary executable format, and a graphical debugger. Designed as an educational and experimental platform for low-level programming.

## ⚠️ Work in Progress

This emulator is still under active development. Things that may go wrong:

- Some instructions are missing or may work incorrectly.
- Interrupts and I/O ports are not fully wired up — keyboard input may not work as expected.
- Programs that run fine today might break after an update.
- You may encounter crashes, freezes, or unexpected behavior.

These issues are being worked on. The goal is to stabilize the instruction set and complete the I/O subsystem before the first release.

If something doesn't work, please report it in [Issues](https://github.com/torutos/tpf-t16xe/issues). If you'd rather wait for a polished version, the first release is on the way.

## Features

### CPU Emulator
- 1 MB addressable memory space
- 5 general-purpose 16-bit registers: AX, BX, CX, DX, SP
- Segment registers: CS (code), DS (data), SS (stack)
- Flags: Carry (C), Zero (Z), Negative (N), Overflow (O), Interrupt Disable (I)
- Hardware interrupts (keyboard via port 0xFFF0/0xFFF1)
- Software interrupts (INT instruction, vector table at 0x0000-0x03FF)
- Stack operations (PUSH, POP, CALL, RET)
- I/O ports for character output (0xFFF2)

### Assembler (TASM)
- Two-pass assembler with label resolution
- Macros with parameters (`%macro`, `%1`, `%2`)
- EQU constants with expression substitution
- Local labels (starting with `.`)
- `times` directive for repetition
- `%include` for multi-file projects
- Data directives: `db`, `dw`, `ds`, `resb`, `resw`, `fill`
- Sections: `.data` and `.code`
- Custom binary format `.toru` with header (data size, code size, origin address)

### GUI Studio (TPSGUI)
- Built with ImGui and GLFW
- Code editor with syntax support (TextEditor)
- Register viewer (AX, BX, CX, DX, SP, PC, flags)
- Memory dump viewer (hex + ASCII)
- Breakpoints (add/remove/clear, pause on hit)
- Step / Run / Stop execution control
- Terminal output panel
- File operations: Open, Save, Save As via native dialogs

### CLI Toolchain
- Single command: `t16xe program.tsm` — validate, assemble, and run
- Output callback support for custom frontends
- Input callback support for keyboard-driven programs

## Building

```bash
cmake -B build
cmake --build build
```

Requirements: C++17, CMake 3.16+, OpenGL (for GUI). All dependencies (ImGui, GLFW) are fetched automatically.

Usage

```bash
# CLI: assemble and run
t16xe program.tsm

# GUI: Toru Processor Studio
tpsgui
```

Examples: [tests/](tests)

Project Structure

```
tpf-t16xe/
├── t16xe/src/kernel/       # CPU emulator core
├── t16xe/src/asm/          # Assembler
├── gui/                    # GUI Studio (ImGui + GLFW)
│   ├── panels/             # Editor, Emulator, Registers, Memory
│   └── libs/               # Third-party libraries
└── tcc/                    # Compiler (planned)
```

Links

· Discord: https://discord.gg/wSbqFkRtE
· Telegram: https://t.me/hobnchkn

License

MIT