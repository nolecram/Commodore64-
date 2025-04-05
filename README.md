# Commodore 64 Emulator

![Commodore 64](https://upload.wikimedia.org/wikipedia/commons/thumb/9/9d/Commodore64.jpg/320px-Commodore64.jpg)

A comprehensive Commodore 64 emulator that runs as an interactive shell environment, allowing you to experience the classic C64 computing environment with accurate hardware emulation.

## Overview

The Commodore 64 was one of the most popular home computers of the 1980s, with an estimated 12.5-17 million units sold. This emulator aims to recreate the C64 experience with accurate hardware emulation including the MOS 6510 CPU, memory banking, and I/O operations.

## Features

- **MOS 6510 CPU Emulation**: Accurate implementation of the 6510 processor with support for most instructions
- **Memory Management**: Full 64KB memory with proper ROM/RAM banking and paging optimization
- **ROM Support**: Ability to load original BASIC, KERNAL, and Character ROMs
- **Basic I/O**: Screen output and keyboard input handling
- **Debugging Tools**: Memory dumps, CPU state inspection, instruction stepping
- **Shell Interface**: Command-line interface with support for both emulator commands and BASIC mode
- **Demo Programs**: Sample programs demonstrating the emulator's capabilities

## System Requirements

- C compiler (GCC recommended)
- Make
- Terminal with ANSI color support (for optimal experience)
- Approximately 5MB disk space

## Building

To build the emulator, simply run:

```bash
make
```

This will compile the emulator and create the executable `c64emu`.

### Build Options

You can use the following make targets:

- `make clean` - Remove compiled files
- `make debug` - Build with debugging symbols
- `make optimized` - Build with optimizations for speed

## Running

After building, you can run the emulator with:

```bash
make run
```

Or directly:

```bash
./c64emu
```

### ROM Files

The emulator will look for the following ROM files in the `roms/` directory:

- `basic.rom` - C64 BASIC ROM (8KB)
- `kernal.rom` - C64 KERNAL ROM (8KB)
- `chargen.rom` - C64 Character Generator ROM (4KB)

If the ROM files are not found, the emulator will use built-in placeholders.

**Note**: Original Commodore 64 ROM files are copyrighted by Commodore Business Machines (now owned by Cloanto). Using actual ROM files may require legal ownership of a real Commodore 64 or proper licensing.

## Shell Commands

The emulator provides a shell interface with the following commands:

| Command | Description |
|---------|-------------|
| `help` | Show available commands |
| `run` | Run the current program |
| `load <file>` | Load a program from a file |
| `list` | List the current BASIC program |
| `dump [addr] [len]` | Dump memory contents (default: 16 bytes) |
| `reset` | Reset the system |
| `step [n]` | Execute n instructions (default: 1) |
| `trace [0\|1]` | Enable/disable instruction tracing |
| `basic` | Enter BASIC mode |
| `poke addr,val` | Write a value to memory address |
| `peek addr` | Read a value from memory address |
| `sys addr` | Call a machine language routine |
| `quit` | Exit the emulator |

## BASIC Mode

When in BASIC mode, you can use a simplified subset of Commodore 64 BASIC commands:

```
READY.
10 PRINT "HELLO WORLD"
20 GOTO 10
RUN
```

Supported commands include:

- `PRINT <text>` - Print text to the screen
- `GOTO <line>` - Jump to specified line number
- `IF <cond> THEN <stmt>` - Conditional execution
- `LET <var>=<expr>` - Assign a value to a variable
- `REM <comment>` - Remark (comment)
- `CLS` - Clear the screen (non-standard C64 command)
- `LIST` - List the program
- `RUN` - Run the program
- `NEW` - Clear the current program
- `exit` or `quit` - Exit BASIC mode and return to the shell

## Memory Map

The Commodore 64 has a complex memory layout with banked ROM and RAM regions:

| Address Range | Description |
|---------------|-------------|
| $0000-$00FF | Zero Page (CPU variables and pointers) |
| $0100-$01FF | Stack |
| $0200-$03FF | System variables and buffers |
| $0400-$07FF | Screen memory (1000 bytes) |
| $0800-$9FFF | BASIC program space |
| $A000-$BFFF | BASIC ROM (can be banked out) |
| $C000-$CFFF | Free RAM (when BASIC ROM is banked out) |
| $D000-$DFFF | I/O or Character ROM (depending on banking) |
| $E000-$FFFF | KERNAL ROM (can be banked out) |

## Demo Programs

The emulator comes with sample programs in the `demos/` directory:

- `hello_world.c` - C source code that creates a simple C64 program
- `hello_world.prg` - Compiled version ready to load in the emulator

To run a demo program:

```
load demos/hello_world.prg
run
```

## Architecture

The emulator is split into several modules:

- **CPU**: MOS 6510 processor emulation
- **Memory**: 64KB address space with proper banking
- **I/O**: Input/output handling
- **Shell**: Command interface

## Performance Optimizations

The emulator includes several performance optimizations:

- Paged memory access for faster memory reads
- Memory banking cache that only updates when configuration changes
- Efficient CPU instruction implementation

## License

This project is licensed under the GNU General Public License v3.0 - see the LICENSE file for details.

## Future Improvements

- Complete MOS 6510 CPU instruction set implementation
- VIC-II graphics emulation for full display capability
- SID sound chip emulation for authentic audio
- Full BASIC interpreter with complete command set
- Proper joystick and peripherals support
- Improved loading and saving of PRG files
- Disk drive emulation (1541)
- Cycle-accurate timing

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Acknowledgments

- Commodore Business Machines for creating the iconic Commodore 64
- The C64 documentation community for detailed hardware specifications
- Everyone who has contributed to understanding C64 internals