/**
 * cpu.h - MOS 6510 CPU emulation for the Commodore 64 emulator
 * 
 * This file contains the core CPU emulation for the Commodore 64's MOS 6510 processor.
 * The 6510 is an 8-bit microprocessor with 16-bit addressing capability, allowing it
 * to address up to 64KB of memory. It's essentially a modified 6502 with an additional
 * I/O port for controlling the memory banking.
 * 
 * Technical specifications:
 * - 8-bit data bus
 * - 16-bit address bus (64KB addressable memory)
 * - 56 instructions with 13 addressing modes
 * - ~1 MHz clock speed in the original Commodore 64
 * 
 * @copyright This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef CPU_H
#define CPU_H

#include <stdint.h>

/**
 * CPU Register Structure
 * 
 * Contains all the registers and flags for the 6510 CPU:
 * - Program Counter (PC): 16-bit register pointing to the next instruction
 * - Accumulator (A): 8-bit general purpose register for arithmetic and logic operations
 * - X, Y: 8-bit index registers for addressing and counting
 * - Stack Pointer (SP): 8-bit offset from $0100, grows downward (0xFF to 0x00)
 * - Status Register (P): Individual status flags for CPU state
 */
typedef struct {
    uint16_t pc;    // Program Counter
    uint8_t a;      // Accumulator
    uint8_t x;      // X Index Register
    uint8_t y;      // Y Index Register
    uint8_t sp;     // Stack Pointer
    
    // Status Register (P) flags
    uint8_t c;      // Carry Flag (bit 0) - Set if operation resulted in carry or borrow
    uint8_t z;      // Zero Flag (bit 1) - Set if result is zero
    uint8_t i;      // Interrupt Disable (bit 2) - Set to disable IRQ interrupts
    uint8_t d;      // Decimal Mode (bit 3) - Set for BCD arithmetic (not used in C64)
    uint8_t b;      // Break Command (bit 4) - Set when BRK instruction executed
    uint8_t v;      // Overflow Flag (bit 6) - Set on arithmetic overflow
    uint8_t n;      // Negative Flag (bit 7) - Set if result is negative (bit 7 is set)
} CPU;

/**
 * Initialize the CPU
 * Sets up initial register values and prepares lookup tables for opcodes
 */
void cpu_init();

/**
 * Get the current CPU status register as a single byte
 * @return Byte representing all status flag bits combined
 */
uint8_t cpu_get_status();

/**
 * Set the CPU status register from a single byte
 * @param status Byte containing all status flag bits
 */
void cpu_set_status(uint8_t status);

/**
 * Execute a single CPU instruction
 * Reads the opcode at the current PC, processes it, and updates CPU state
 */
void cpu_step();

/**
 * Execute multiple CPU instructions
 * @param cycles Approximate number of cycles to execute
 */
void cpu_execute(uint32_t cycles);

/**
 * Trigger an interrupt (IRQ or NMI)
 * @param is_nmi If non-zero, this is a non-maskable interrupt (NMI)
 */
void cpu_interrupt(int is_nmi);

/**
 * Reset the CPU
 * Sets the PC to the address in the reset vector and initializes registers
 */
void cpu_reset();

/**
 * Print the current CPU state for debugging
 * Displays all registers and flags
 */
void cpu_print_state();

/**
 * Set the program counter to a specific address
 * @param address The 16-bit address to set PC to
 */
void cpu_set_pc(uint16_t address);

/**
 * Emulate KERNAL ROM functions
 * This provides implementations for key C64 KERNAL routines
 * @param address The address of the KERNAL function to emulate
 */
void cpu_emulate_kernal(uint16_t address);

/**
 * Addressing Modes for 6510 Instructions
 *
 * These modes determine how the CPU calculates the effective address
 * for memory operations. Each instruction uses a specific addressing mode.
 */
typedef enum {
    ADDR_IMPLIED,          // No operand (e.g., RTS)
    ADDR_ACCUMULATOR,      // Operations on accumulator (e.g., LSR A)
    ADDR_IMMEDIATE,        // Immediate value (e.g., LDA #$10)
    ADDR_ZERO_PAGE,        // Zero page address (e.g., LDA $10)
    ADDR_ZERO_PAGE_X,      // Zero page with X offset (e.g., LDA $10,X)
    ADDR_ZERO_PAGE_Y,      // Zero page with Y offset (e.g., LDX $10,Y)
    ADDR_RELATIVE,         // Relative offset for branches (e.g., BEQ label)
    ADDR_ABSOLUTE,         // Full 16-bit address (e.g., JMP $1234)
    ADDR_ABSOLUTE_X,       // Absolute with X offset (e.g., LDA $1234,X)
    ADDR_ABSOLUTE_Y,       // Absolute with Y offset (e.g., LDA $1234,Y)
    ADDR_INDIRECT,         // Indirect JMP (e.g., JMP ($1234))
    ADDR_INDEXED_INDIRECT, // Indexed indirect (e.g., LDA ($10,X))
    ADDR_INDIRECT_INDEXED  // Indirect indexed (e.g., LDA ($10),Y)
} AddressingMode;

/**
 * Special Memory Locations
 * 
 * These are key addresses in the 6510 memory map
 */
#define STACK_PAGE 0x0100   // Stack is always at page $01 (0x0100-0x01FF)
#define NMI_VECTOR 0xFFFA   // Non-maskable interrupt vector (2 bytes)
#define RESET_VECTOR 0xFFFC // Reset vector (2 bytes)
#define IRQ_VECTOR 0xFFFE   // IRQ/BRK vector (2 bytes)

#endif /* CPU_H */