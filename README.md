# T16XE — 16-bit CPU Emulator & Toolchain

A fully custom 16-bit processor emulator written in C++17, featuring its own instruction set, assembler, binary executable format, and a graphical debugger. Designed as an educational and experimental platform for low-level programming.

## ⚠️ Work in Progress

This emulator is still under active development. We are currently performing a large-scale refactoring of the core to fix known bugs, improve stability, and prepare for the first release.

Things that may go wrong right now:
- Some instructions are missing or may work incorrectly.
- Interrupts and I/O ports are being stabilized — keyboard input may not work as expected.
- Programs that run fine today might break after an update.
- You may encounter crashes, freezes, or unexpected behavior.

If something doesn't work, please report it in [Issues](https://github.com/torutos/tpf-t16xe/issues).

## Features

### CPU Emulator
- 32 MB addressable memory space
- 5 general-purpose 16-bit registers: AX, BX, CX, DX, SP
- Segment registers: CS (code), DS (data), SS (stack)
- Flags: Carry (C), Zero (Z), Negative (N), Overflow (O), Interrupt Disable (I)
- Hardware interrupts (keyboard via port 0xFFF0/0xFFF1)
- Software interrupts (INT instruction, vector table at 0x10000-0x103FF)
- Stack operations (PUSH, POP, CALL, RET — far calls supported)
- I/O ports for character output (0x1FFFFF2)

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

## Documentation & Development

We have started improving the project's documentation to make it easier to understand, contribute, and track progress. 

- **`docs/`** — Project documentation, design notes, and architecture decisions.
- **`docs/TODO.md`** — Known bugs, planned features, and technical debt. 
- **`docs/opcodes.md`** — Instruction set reference and opcode map.
- **Bugfixes & History** — We are actively fixing core bugs (e.g., stack symmetry, far calls, flag updates). The commit history and the `TODO` list are the primary sources of truth for what has been fixed and what is still pending.

*(More detailed design documents will be added as the refactoring progresses.)*

## Building

```bash
cmake -B build
cmake --build build
```

Requirements: C++17, CMake 3.16+, OpenGL (for GUI). All dependencies (ImGui, GLFW) are fetched automatically.

## Usage

```bash
# CLI: assemble and run
t16xe program.tsm

# GUI: Toru Processor Studio
tpsgui
```

Examples: [tests](tests)

## Links

· Discord: https://discord.gg/wSbqFkRtE
· Telegram: https://t.me/hobnchkn

## License

AGPL-3.0