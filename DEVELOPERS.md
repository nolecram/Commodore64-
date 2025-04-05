# Commodore 64 Emulator Developer Guide

This document provides technical information for developers who want to understand or contribute to the Commodore 64 emulator project.

## Project Architecture

The emulator is organized into four main subsystems:

1. **CPU Emulation** (`src/cpu/`) - Emulates the MOS 6510 processor
2. **Memory Management** (`src/memory/`) - Handles the 64KB memory space with banking
3. **I/O Operations** (`src/io/`) - Manages input/output operations
4. **Shell Interface** (`src/shell/`) - Provides the user interface and command processing

The main program (`src/main.c`) coordinates these subsystems and initializes the emulator.

## Build System

The project uses a simple Makefile build system. The main targets are:

- `make` - Builds the emulator
- `make clean` - Removes object files and executable
- `make run` - Builds and runs the emulator

## Memory Architecture

### Memory Banking

The C64 has a complex memory banking system that allows overlapping memory regions:

```
$FFFF +-------------+
      | KERNAL ROM  | 8KB (can be replaced by RAM)
$E000 +-------------+
      | Free RAM    | 4KB (when I/O is disabled)
$D000 +-------------+
      | I/O / CHAR  | 4KB (I/O registers or character ROM or RAM)
$C000 +-------------+
      | Free RAM    | 4KB (when BASIC ROM is disabled)
$A000 +-------------+
      | BASIC ROM   | 8KB (can be replaced by RAM)
$8000 +-------------+
      |             |
      | BASIC RAM   | 32KB (user program space)
      |             |
$0800 +-------------+
      | System Area | 2KB (screen memory, vectors, etc.)
$0000 +-------------+
```

The memory banking is controlled by the processor port at address $0001:

- Bit 0: BASIC ROM enable (1=enabled)
- Bit 1: KERNAL ROM enable (1=enabled)
- Bit 2: Character ROM/I/O select (0=Character ROM, 1=I/O)

### Memory Optimization

The emulator uses paged memory access for performance:

1. Memory is organized into 256 pages of 256 bytes each
2. A lookup table maps each page to the appropriate memory region
3. Memory banking updates are performed only when configuration changes

## CPU Emulation

The MOS 6510 CPU emulation handles:

1. Register state (A, X, Y, SP, PC, flags)
2. Instruction decoding and execution
3. Addressing mode resolution
4. Interrupts (NMI, IRQ)
5. KERNAL ROM routine emulation

### Instruction Implementation

Each CPU instruction is implemented with:

1. Addressing mode resolution
2. Instruction execution
3. CPU state update (flags, registers)
4. Cycle counting

Example instruction execution:

```c
// LDA (Load Accumulator) - Loads a value into the accumulator
case 0xA9:  // LDA Immediate
    cpu.a = memory_read(address);
    cpu.z = (cpu.a == 0);         // Set Zero flag if result is zero
    cpu.n = (cpu.a & 0x80) != 0;  // Set Negative flag if bit 7 is set
    break;
```

## Adding New Features

### Implementing Additional CPU Instructions

To add a new CPU instruction:

1. Add the opcode to the instruction tables in `cpu_init()`
2. Implement the instruction in the switch statement in `cpu_step()`
3. Test with a small program that uses the instruction

### Adding I/O Device Support

To implement a new I/O device:

1. Add memory-mapped registers to the I/O region ($D000-$DFFF)
2. Add special handling for these registers in `memory_read()` and `memory_write()`
3. Implement the device behavior in response to register changes

### Implementing VIC-II Graphics

The VIC-II video chip can be implemented by:

1. Adding VIC registers in the I/O region ($D000-$D3FF)
2. Creating a screen rendering system in the I/O module
3. Implementing sprite and character display logic
4. Adding terminal or SDL-based graphics output

### Adding SID Sound Support

The SID sound chip can be implemented by:

1. Adding SID registers in the I/O region ($D400-$D7FF)
2. Creating a sound synthesis system
3. Implementing the various sound waveforms and filters
4. Adding audio output through an audio library (e.g., SDL_mixer)

## Debugging

The emulator includes several debugging features:

1. Memory dumping with the `dump` command
2. CPU state inspection with CPU flags display
3. Instruction tracing
4. Single-step execution

## Performance Considerations

For optimal emulator performance:

1. Use lookup tables for frequently accessed data
2. Minimize conditional branches in hot paths
3. Consider block-based execution for faster emulation
4. Use profile-guided optimization when compiling with `gcc -pg`

## Code Style Guidelines

When contributing to the project, please follow these guidelines:

1. Use clear function and variable names
2. Add comprehensive comments for complex logic
3. Document all public functions with parameters and return values
4. Use consistent indentation (4 spaces)
5. Group related functionality together

## Resources

Helpful resources for Commodore 64 development:

- [C64 Memory Map](https://sta.c64.org/cbm64mem.html)
- [6502/6510 Instruction Set](https://www.masswerk.at/6502/6502_instruction_set.html)
- [C64 Programmer's Reference Guide](https://www.commodore.ca/manuals/c64_programmers_reference/c64-programmers_reference.htm)
- [VIC-II Documentation](https://www.cebix.net/VIC-Article.txt)
- [SID Documentation](https://www.sidmusic.org/sid/sidtech1.html)