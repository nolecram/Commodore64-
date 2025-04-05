/**
 * cpu.c
 * MOS 6510 CPU emulation implementation for the Commodore 64 emulator
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include "cpu.h"
#include "../memory/memory.h"

// CPU state
static CPU cpu;
static uint32_t cycles = 0;

// Lookup tables for opcodes
static uint8_t opcode_sizes[256];
static uint8_t opcode_cycles[256];
static AddressingMode opcode_modes[256];

// Internal function declarations
static uint16_t cpu_get_operand_address(AddressingMode mode);
static void cpu_push_byte(uint8_t value);
static uint8_t cpu_pull_byte();
static void cpu_push_word(uint16_t value);
static uint16_t cpu_pull_word();

/**
 * Check if a key has been pressed (non-blocking)
 * This is a simplified implementation - a real one would use platform-specific code
 */
static int kbhit() {
#ifdef _WIN32
    // Windows implementation would go here
    return 0;
#elif defined(__APPLE__) || defined(__unix__) || defined(__unix) || defined(unix)
    // Unix-like systems (macOS, Linux)
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    
    // Use select to check if input is available
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
#else
    // Fallback implementation
    return 0;
#endif
}

/**
 * Initialize the CPU
 */
void cpu_init() {
    // Reset CPU registers
    cpu.pc = 0;
    cpu.a = 0;
    cpu.x = 0;
    cpu.y = 0;
    cpu.sp = 0xFD;  // Initial stack pointer value
    
    // Reset CPU flags
    cpu.c = 0;
    cpu.z = 0;
    cpu.i = 1;  // Interrupts disabled at startup
    cpu.d = 0;
    cpu.b = 0;
    cpu.v = 0;
    cpu.n = 0;
    
    // Initialize opcode tables with default values
    memset(opcode_sizes, 1, sizeof(opcode_sizes));
    memset(opcode_cycles, 2, sizeof(opcode_cycles));
    memset(opcode_modes, ADDR_IMPLIED, sizeof(opcode_modes));
    
    // Set up instruction sizes, cycle counts, and addressing modes for implemented opcodes
    
    // LDA - Load Accumulator
    opcode_sizes[0xA9] = 2; opcode_cycles[0xA9] = 2; opcode_modes[0xA9] = ADDR_IMMEDIATE;     // LDA Immediate
    opcode_sizes[0xA5] = 2; opcode_cycles[0xA5] = 3; opcode_modes[0xA5] = ADDR_ZERO_PAGE;     // LDA Zero Page
    opcode_sizes[0xB5] = 2; opcode_cycles[0xB5] = 4; opcode_modes[0xB5] = ADDR_ZERO_PAGE_X;   // LDA Zero Page,X
    opcode_sizes[0xAD] = 3; opcode_cycles[0xAD] = 4; opcode_modes[0xAD] = ADDR_ABSOLUTE;      // LDA Absolute
    opcode_sizes[0xBD] = 3; opcode_cycles[0xBD] = 4; opcode_modes[0xBD] = ADDR_ABSOLUTE_X;    // LDA Absolute,X
    opcode_sizes[0xB9] = 3; opcode_cycles[0xB9] = 4; opcode_modes[0xB9] = ADDR_ABSOLUTE_Y;    // LDA Absolute,Y
    opcode_sizes[0xA1] = 2; opcode_cycles[0xA1] = 6; opcode_modes[0xA1] = ADDR_INDEXED_INDIRECT;  // LDA (Indirect,X)
    opcode_sizes[0xB1] = 2; opcode_cycles[0xB1] = 5; opcode_modes[0xB1] = ADDR_INDIRECT_INDEXED;  // LDA (Indirect),Y
    
    // LDX - Load X Register
    opcode_sizes[0xA2] = 2; opcode_cycles[0xA2] = 2; opcode_modes[0xA2] = ADDR_IMMEDIATE;     // LDX Immediate
    opcode_sizes[0xA6] = 2; opcode_cycles[0xA6] = 3; opcode_modes[0xA6] = ADDR_ZERO_PAGE;     // LDX Zero Page
    opcode_sizes[0xB6] = 2; opcode_cycles[0xB6] = 4; opcode_modes[0xB6] = ADDR_ZERO_PAGE_Y;   // LDX Zero Page,Y
    opcode_sizes[0xAE] = 3; opcode_cycles[0xAE] = 4; opcode_modes[0xAE] = ADDR_ABSOLUTE;      // LDX Absolute
    opcode_sizes[0xBE] = 3; opcode_cycles[0xBE] = 4; opcode_modes[0xBE] = ADDR_ABSOLUTE_Y;    // LDX Absolute,Y
    
    // LDY - Load Y Register
    opcode_sizes[0xA0] = 2; opcode_cycles[0xA0] = 2; opcode_modes[0xA0] = ADDR_IMMEDIATE;     // LDY Immediate
    opcode_sizes[0xA4] = 2; opcode_cycles[0xA4] = 3; opcode_modes[0xA4] = ADDR_ZERO_PAGE;     // LDY Zero Page
    opcode_sizes[0xB4] = 2; opcode_cycles[0xB4] = 4; opcode_modes[0xB4] = ADDR_ZERO_PAGE_X;   // LDY Zero Page,X
    opcode_sizes[0xAC] = 3; opcode_cycles[0xAC] = 4; opcode_modes[0xAC] = ADDR_ABSOLUTE;      // LDY Absolute
    opcode_sizes[0xBC] = 3; opcode_cycles[0xBC] = 4; opcode_modes[0xBC] = ADDR_ABSOLUTE_X;    // LDY Absolute,X
    
    // STA - Store Accumulator
    opcode_sizes[0x85] = 2; opcode_cycles[0x85] = 3; opcode_modes[0x85] = ADDR_ZERO_PAGE;     // STA Zero Page
    opcode_sizes[0x95] = 2; opcode_cycles[0x95] = 4; opcode_modes[0x95] = ADDR_ZERO_PAGE_X;   // STA Zero Page,X
    opcode_sizes[0x8D] = 3; opcode_cycles[0x8D] = 4; opcode_modes[0x8D] = ADDR_ABSOLUTE;      // STA Absolute
    opcode_sizes[0x9D] = 3; opcode_cycles[0x9D] = 5; opcode_modes[0x9D] = ADDR_ABSOLUTE_X;    // STA Absolute,X
    opcode_sizes[0x99] = 3; opcode_cycles[0x99] = 5; opcode_modes[0x99] = ADDR_ABSOLUTE_Y;    // STA Absolute,Y
    opcode_sizes[0x81] = 2; opcode_cycles[0x81] = 6; opcode_modes[0x81] = ADDR_INDEXED_INDIRECT;  // STA (Indirect,X)
    opcode_sizes[0x91] = 2; opcode_cycles[0x91] = 6; opcode_modes[0x91] = ADDR_INDIRECT_INDEXED;  // STA (Indirect),Y
    
    // STX - Store X Register
    opcode_sizes[0x86] = 2; opcode_cycles[0x86] = 3; opcode_modes[0x86] = ADDR_ZERO_PAGE;     // STX Zero Page
    opcode_sizes[0x96] = 2; opcode_cycles[0x96] = 4; opcode_modes[0x96] = ADDR_ZERO_PAGE_Y;   // STX Zero Page,Y
    opcode_sizes[0x8E] = 3; opcode_cycles[0x8E] = 4; opcode_modes[0x8E] = ADDR_ABSOLUTE;      // STX Absolute
    
    // STY - Store Y Register
    opcode_sizes[0x84] = 2; opcode_cycles[0x84] = 3; opcode_modes[0x84] = ADDR_ZERO_PAGE;     // STY Zero Page
    opcode_sizes[0x94] = 2; opcode_cycles[0x94] = 4; opcode_modes[0x94] = ADDR_ZERO_PAGE_X;   // STY Zero Page,X
    opcode_sizes[0x8C] = 3; opcode_cycles[0x8C] = 4; opcode_modes[0x8C] = ADDR_ABSOLUTE;      // STY Absolute
    
    // JMP - Jump
    opcode_sizes[0x4C] = 3; opcode_cycles[0x4C] = 3; opcode_modes[0x4C] = ADDR_ABSOLUTE;      // JMP Absolute
    opcode_sizes[0x6C] = 3; opcode_cycles[0x6C] = 5; opcode_modes[0x6C] = ADDR_INDIRECT;      // JMP Indirect
    
    // JSR/RTS - Subroutine operations
    opcode_sizes[0x20] = 3; opcode_cycles[0x20] = 6; opcode_modes[0x20] = ADDR_ABSOLUTE;      // JSR Absolute
    opcode_sizes[0x60] = 1; opcode_cycles[0x60] = 6; opcode_modes[0x60] = ADDR_IMPLIED;       // RTS Implied
    
    // Register increment/decrement
    opcode_sizes[0xE8] = 1; opcode_cycles[0xE8] = 2; opcode_modes[0xE8] = ADDR_IMPLIED;       // INX Implied
    opcode_sizes[0xC8] = 1; opcode_cycles[0xC8] = 2; opcode_modes[0xC8] = ADDR_IMPLIED;       // INY Implied
    opcode_sizes[0xCA] = 1; opcode_cycles[0xCA] = 2; opcode_modes[0xCA] = ADDR_IMPLIED;       // DEX Implied
    opcode_sizes[0x88] = 1; opcode_cycles[0x88] = 2; opcode_modes[0x88] = ADDR_IMPLIED;       // DEY Implied
    
    // CMP - Compare Accumulator
    opcode_sizes[0xC9] = 2; opcode_cycles[0xC9] = 2; opcode_modes[0xC9] = ADDR_IMMEDIATE;     // CMP Immediate
    opcode_sizes[0xC5] = 2; opcode_cycles[0xC5] = 3; opcode_modes[0xC5] = ADDR_ZERO_PAGE;     // CMP Zero Page
    opcode_sizes[0xD5] = 2; opcode_cycles[0xD5] = 4; opcode_modes[0xD5] = ADDR_ZERO_PAGE_X;   // CMP Zero Page,X
    opcode_sizes[0xCD] = 3; opcode_cycles[0xCD] = 4; opcode_modes[0xCD] = ADDR_ABSOLUTE;      // CMP Absolute
    opcode_sizes[0xDD] = 3; opcode_cycles[0xDD] = 4; opcode_modes[0xDD] = ADDR_ABSOLUTE_X;    // CMP Absolute,X
    opcode_sizes[0xD9] = 3; opcode_cycles[0xD9] = 4; opcode_modes[0xD9] = ADDR_ABSOLUTE_Y;    // CMP Absolute,Y
    opcode_sizes[0xC1] = 2; opcode_cycles[0xC1] = 6; opcode_modes[0xC1] = ADDR_INDEXED_INDIRECT; // CMP (Indirect,X)
    opcode_sizes[0xD1] = 2; opcode_cycles[0xD1] = 5; opcode_modes[0xD1] = ADDR_INDIRECT_INDEXED; // CMP (Indirect),Y
    
    // Branch instructions
    opcode_sizes[0xF0] = 2; opcode_cycles[0xF0] = 2; opcode_modes[0xF0] = ADDR_RELATIVE;      // BEQ Relative
    opcode_sizes[0xD0] = 2; opcode_cycles[0xD0] = 2; opcode_modes[0xD0] = ADDR_RELATIVE;      // BNE Relative
    opcode_sizes[0xB0] = 2; opcode_cycles[0xB0] = 2; opcode_modes[0xB0] = ADDR_RELATIVE;      // BCS Relative
    opcode_sizes[0x90] = 2; opcode_cycles[0x90] = 2; opcode_modes[0x90] = ADDR_RELATIVE;      // BCC Relative
    opcode_sizes[0x30] = 2; opcode_cycles[0x30] = 2; opcode_modes[0x30] = ADDR_RELATIVE;      // BMI Relative
    opcode_sizes[0x10] = 2; opcode_cycles[0x10] = 2; opcode_modes[0x10] = ADDR_RELATIVE;      // BPL Relative
    opcode_sizes[0x70] = 2; opcode_cycles[0x70] = 2; opcode_modes[0x70] = ADDR_RELATIVE;      // BVS Relative
    opcode_sizes[0x50] = 2; opcode_cycles[0x50] = 2; opcode_modes[0x50] = ADDR_RELATIVE;      // BVC Relative
    
    // Reset the CPU
    cpu_reset();
}

/**
 * Pack the status register flags into a single byte
 */
uint8_t cpu_get_status() {
    uint8_t status = 0;
    status |= cpu.c << 0;
    status |= cpu.z << 1;
    status |= cpu.i << 2;
    status |= cpu.d << 3;
    status |= cpu.b << 4;
    status |= 1 << 5;     // Bit 5 is always set
    status |= cpu.v << 6;
    status |= cpu.n << 7;
    return status;
}

/**
 * Unpack a status byte into the individual flag bits
 */
void cpu_set_status(uint8_t status) {
    cpu.c = (status >> 0) & 1;
    cpu.z = (status >> 1) & 1;
    cpu.i = (status >> 2) & 1;
    cpu.d = (status >> 3) & 1;
    cpu.b = (status >> 4) & 1;
    // Bit 5 is ignored
    cpu.v = (status >> 6) & 1;
    cpu.n = (status >> 7) & 1;
}

/**
 * Reset the CPU - load the reset vector and start execution
 */
void cpu_reset() {
    // Read the reset vector from 0xFFFC-0xFFFD
    uint16_t reset_vector = memory_read(RESET_VECTOR) | (memory_read(RESET_VECTOR + 1) << 8);
    
    // Set the program counter to the reset vector
    cpu.pc = reset_vector;
    
    // Set the initial status
    cpu.sp = 0xFD;  // Reset stack pointer
    cpu.i = 1;      // Disable interrupts
    
    // Reset cycle count
    cycles = 0;
}

/**
 * Push a byte to the stack
 */
static void cpu_push_byte(uint8_t value) {
    memory_write(STACK_PAGE + cpu.sp, value);
    cpu.sp--;  // Stack grows downward
}

/**
 * Pull a byte from the stack
 */
static uint8_t cpu_pull_byte() {
    cpu.sp++;  // Stack grows downward
    return memory_read(STACK_PAGE + cpu.sp);
}

/**
 * Push a 16-bit word to the stack (high byte first)
 */
static void cpu_push_word(uint16_t value) {
    cpu_push_byte((value >> 8) & 0xFF);  // High byte
    cpu_push_byte(value & 0xFF);         // Low byte
}

/**
 * Pull a 16-bit word from the stack (low byte first)
 */
static uint16_t cpu_pull_word() {
    uint8_t low = cpu_pull_byte();
    uint8_t high = cpu_pull_byte();
    return (high << 8) | low;
}

/**
 * Handle a CPU interrupt (NMI or IRQ)
 */
void cpu_interrupt(int is_nmi) {
    // Interrupts are handled differently if they're NMI (non-maskable)
    if (is_nmi || !cpu.i) {
        // Push the return address to the stack
        cpu_push_word(cpu.pc);
        
        // Push the status with B flag cleared
        uint8_t status = cpu_get_status();
        status &= ~(1 << 4);  // Clear B flag
        cpu_push_byte(status);
        
        // Set the interrupt disable flag
        cpu.i = 1;
        
        // Load the interrupt vector
        uint16_t vector = is_nmi ? NMI_VECTOR : IRQ_VECTOR;
        cpu.pc = memory_read(vector) | (memory_read(vector + 1) << 8);
        
        // Add cycles
        cycles += 7;
    }
}

/**
 * Get the effective address based on the addressing mode
 */
static uint16_t cpu_get_operand_address(AddressingMode mode) {
    uint16_t address = 0;
    
    switch (mode) {
        case ADDR_IMPLIED:
        case ADDR_ACCUMULATOR:
            // No address for these modes
            return 0;
            
        case ADDR_IMMEDIATE:
            // The operand is the next byte
            return cpu.pc + 1;
            
        case ADDR_ZERO_PAGE:
            // Zero page addressing - address is the next byte
            return memory_read(cpu.pc + 1);
            
        case ADDR_ZERO_PAGE_X:
            // Zero page,X addressing - address is the next byte + X
            return (memory_read(cpu.pc + 1) + cpu.x) & 0xFF;
            
        case ADDR_ZERO_PAGE_Y:
            // Zero page,Y addressing - address is the next byte + Y
            return (memory_read(cpu.pc + 1) + cpu.y) & 0xFF;
            
        case ADDR_RELATIVE:
            // Relative addressing - for branch instructions
            {
                int8_t offset = (int8_t)memory_read(cpu.pc + 1);
                return cpu.pc + 2 + offset;
            }
            
        case ADDR_ABSOLUTE:
            // Absolute addressing - address is the next two bytes
            return memory_read(cpu.pc + 1) | (memory_read(cpu.pc + 2) << 8);
            
        case ADDR_ABSOLUTE_X:
            // Absolute,X addressing - address is the next two bytes + X
            return (memory_read(cpu.pc + 1) | (memory_read(cpu.pc + 2) << 8)) + cpu.x;
            
        case ADDR_ABSOLUTE_Y:
            // Absolute,Y addressing - address is the next two bytes + Y
            return (memory_read(cpu.pc + 1) | (memory_read(cpu.pc + 2) << 8)) + cpu.y;
            
        case ADDR_INDIRECT:
            // Indirect addressing - used by JMP
            {
                uint16_t ptr = memory_read(cpu.pc + 1) | (memory_read(cpu.pc + 2) << 8);
                // Simulate the 6502 indirect jump bug at page boundaries
                if ((ptr & 0xFF) == 0xFF) {
                    return memory_read(ptr) | (memory_read(ptr & 0xFF00) << 8);
                } else {
                    return memory_read(ptr) | (memory_read(ptr + 1) << 8);
                }
            }
            
        case ADDR_INDEXED_INDIRECT:
            // (Indirect,X) addressing
            {
                uint8_t zp = (memory_read(cpu.pc + 1) + cpu.x) & 0xFF;
                return memory_read(zp) | (memory_read((zp + 1) & 0xFF) << 8);
            }
            
        case ADDR_INDIRECT_INDEXED:
            // (Indirect),Y addressing
            {
                uint8_t zp = memory_read(cpu.pc + 1);
                return (memory_read(zp) | (memory_read((zp + 1) & 0xFF) << 8)) + cpu.y;
            }
            
        default:
            printf("Error: Unknown addressing mode %d\n", mode);
            return 0;
    }
}

/**
 * Execute a single CPU instruction
 */
void cpu_step() {
    // Read the opcode
    uint8_t opcode = memory_read(cpu.pc);
    
    // Get the addressing mode and instruction size
    AddressingMode mode = opcode_modes[opcode];
    uint8_t size = opcode_sizes[opcode];
    
    // Get the operand address based on the addressing mode
    uint16_t address = cpu_get_operand_address(mode);
    
    // Execute the instruction based on the opcode
    // This is a more expanded implementation with additional opcodes
    switch (opcode) {
        // LDA - Load Accumulator
        case 0xA9:  // LDA Immediate
        case 0xA5:  // LDA Zero Page
        case 0xB5:  // LDA Zero Page,X
        case 0xAD:  // LDA Absolute
        case 0xBD:  // LDA Absolute,X
        case 0xB9:  // LDA Absolute,Y
        case 0xA1:  // LDA (Indirect,X)
        case 0xB1:  // LDA (Indirect),Y
            cpu.a = memory_read(address);
            cpu.z = (cpu.a == 0);
            cpu.n = (cpu.a & 0x80) != 0;
            break;
            
        // LDX - Load X Register
        case 0xA2:  // LDX Immediate
        case 0xA6:  // LDX Zero Page
        case 0xB6:  // LDX Zero Page,Y
        case 0xAE:  // LDX Absolute
        case 0xBE:  // LDX Absolute,Y
            cpu.x = memory_read(address);
            cpu.z = (cpu.x == 0);
            cpu.n = (cpu.x & 0x80) != 0;
            break;
            
        // LDY - Load Y Register
        case 0xA0:  // LDY Immediate
        case 0xA4:  // LDY Zero Page
        case 0xB4:  // LDY Zero Page,X
        case 0xAC:  // LDY Absolute
        case 0xBC:  // LDY Absolute,X
            cpu.y = memory_read(address);
            cpu.z = (cpu.y == 0);
            cpu.n = (cpu.y & 0x80) != 0;
            break;
            
        // STA - Store Accumulator
        case 0x85:  // STA Zero Page
        case 0x95:  // STA Zero Page,X
        case 0x8D:  // STA Absolute
        case 0x9D:  // STA Absolute,X
        case 0x99:  // STA Absolute,Y
        case 0x81:  // STA (Indirect,X)
        case 0x91:  // STA (Indirect),Y
            memory_write(address, cpu.a);
            break;
            
        // STX - Store X Register
        case 0x86:  // STX Zero Page
        case 0x96:  // STX Zero Page,Y
        case 0x8E:  // STX Absolute
            memory_write(address, cpu.x);
            break;
            
        // STY - Store Y Register
        case 0x84:  // STY Zero Page
        case 0x94:  // STY Zero Page,X
        case 0x8C:  // STY Absolute
            memory_write(address, cpu.y);
            break;
            
        // JMP - Jump
        case 0x4C:  // JMP Absolute
        case 0x6C:  // JMP Indirect
            cpu.pc = address;
            size = 0;  // Don't increment PC
            break;
            
        // JSR - Jump to Subroutine
        case 0x20:  // JSR Absolute
            // Check if this is a KERNAL ROM call
            if (address >= 0xFF00) {
                // This is a KERNAL ROM call, handle it directly
                cpu_push_word(cpu.pc + 2);  // Push return address
                cpu_emulate_kernal(address);
                size = 0;  // Don't increment PC
            } else {
                // Standard JSR implementation
                // Push the return address - 1 (PC + 2 - 1 = PC + 1)
                cpu_push_word(cpu.pc + 2 - 1);
                cpu.pc = address;
                size = 0;  // Don't increment PC
            }
            break;
            
        // RTS - Return from Subroutine
        case 0x60:  // RTS Implied
            cpu.pc = cpu_pull_word() + 1;
            size = 0;  // Don't increment PC
            break;
            
        // INX - Increment X Register
        case 0xE8:  // INX Implied
            cpu.x++;
            cpu.z = (cpu.x == 0);
            cpu.n = (cpu.x & 0x80) != 0;
            break;
            
        // INY - Increment Y Register
        case 0xC8:  // INY Implied
            cpu.y++;
            cpu.z = (cpu.y == 0);
            cpu.n = (cpu.y & 0x80) != 0;
            break;
            
        // DEX - Decrement X Register
        case 0xCA:  // DEX Implied
            cpu.x--;
            cpu.z = (cpu.x == 0);
            cpu.n = (cpu.x & 0x80) != 0;
            break;
            
        // DEY - Decrement Y Register
        case 0x88:  // DEY Implied
            cpu.y--;
            cpu.z = (cpu.y == 0);
            cpu.n = (cpu.y & 0x80) != 0;
            break;
            
        // CMP - Compare Accumulator
        case 0xC9:  // CMP Immediate
        case 0xC5:  // CMP Zero Page
        case 0xD5:  // CMP Zero Page,X
        case 0xCD:  // CMP Absolute
        case 0xDD:  // CMP Absolute,X
        case 0xD9:  // CMP Absolute,Y
        case 0xC1:  // CMP (Indirect,X)
        case 0xD1:  // CMP (Indirect),Y
            {
                uint8_t value = memory_read(address);
                uint8_t result = cpu.a - value;
                cpu.c = (cpu.a >= value);
                cpu.z = (result == 0);
                cpu.n = (result & 0x80) != 0;
            }
            break;
            
        // BEQ - Branch if Equal (Z=1)
        case 0xF0:  // BEQ Relative
            if (cpu.z) {
                cpu.pc = address;
                size = 0;  // Don't increment PC
            }
            break;
            
        // BNE - Branch if Not Equal (Z=0)
        case 0xD0:  // BNE Relative
            if (!cpu.z) {
                cpu.pc = address;
                size = 0;  // Don't increment PC
            }
            break;
            
        // BCS - Branch if Carry Set (C=1)
        case 0xB0:  // BCS Relative
            if (cpu.c) {
                cpu.pc = address;
                size = 0;  // Don't increment PC
            }
            break;
            
        // BCC - Branch if Carry Clear (C=0)
        case 0x90:  // BCC Relative
            if (!cpu.c) {
                cpu.pc = address;
                size = 0;  // Don't increment PC
            }
            break;
            
        // BMI - Branch if Minus (N=1)
        case 0x30:  // BMI Relative
            if (cpu.n) {
                cpu.pc = address;
                size = 0;  // Don't increment PC
            }
            break;
            
        // BPL - Branch if Plus (N=0)
        case 0x10:  // BPL Relative
            if (!cpu.n) {
                cpu.pc = address;
                size = 0;  // Don't increment PC
            }
            break;
            
        // BVS - Branch if Overflow Set (V=1)
        case 0x70:  // BVS Relative
            if (cpu.v) {
                cpu.pc = address;
                size = 0;  // Don't increment PC
            }
            break;
            
        // BVC - Branch if Overflow Clear (V=0)
        case 0x50:  // BVC Relative
            if (!cpu.v) {
                cpu.pc = address;
                size = 0;  // Don't increment PC
            }
            break;
        
        // TAX - Transfer Accumulator to X
        case 0xAA:  // TAX Implied
            cpu.x = cpu.a;
            cpu.z = (cpu.x == 0);
            cpu.n = (cpu.x & 0x80) != 0;
            break;
            
        // TAY - Transfer Accumulator to Y
        case 0xA8:  // TAY Implied
            cpu.y = cpu.a;
            cpu.z = (cpu.y == 0);
            cpu.n = (cpu.y & 0x80) != 0;
            break;
            
        // TXA - Transfer X to Accumulator
        case 0x8A:  // TXA Implied
            cpu.a = cpu.x;
            cpu.z = (cpu.a == 0);
            cpu.n = (cpu.a & 0x80) != 0;
            break;
            
        // TYA - Transfer Y to Accumulator
        case 0x98:  // TYA Implied
            cpu.a = cpu.y;
            cpu.z = (cpu.a == 0);
            cpu.n = (cpu.a & 0x80) != 0;
            break;
            
        // TSX - Transfer Stack Pointer to X
        case 0xBA:  // TSX Implied
            cpu.x = cpu.sp;
            cpu.z = (cpu.x == 0);
            cpu.n = (cpu.x & 0x80) != 0;
            break;
            
        // TXS - Transfer X to Stack Pointer
        case 0x9A:  // TXS Implied
            cpu.sp = cpu.x;
            break;
            
        default:
            if (opcode == 0x20) {
                cpu_emulate_kernal(address);
            } else {
                printf("Unimplemented opcode: $%02X at $%04X\n", opcode, cpu.pc);
            }
            break;
    }
    
    // Update the program counter and cycle count
    if (size > 0) {
        cpu.pc += size;
    }
    cycles += opcode_cycles[opcode];
}

/**
 * Emulate KERNAL ROM routines
 */
void cpu_emulate_kernal(uint16_t address) {
    switch (address) {
        case 0xFFD2:  // CHROUT - Output a character to the current output device
            {
                // Character is in the accumulator
                char c = cpu.a;
                // Print the character to the screen
                printf("%c", c);
                // In a more advanced implementation, this would update the screen memory
                // and handle control characters properly
            }
            break;
            
        case 0xFFCF:  // CHRIN - Get a character from the current input device
            {
                // Simple implementation - just return a dummy character
                cpu.a = getchar();
            }
            break;
            
        case 0xFFE4:  // GETIN - Check if a key has been pressed
            {
                // Simple implementation - just check stdin for a key
                int c = -1;
                // Check if there's a character available without blocking
                if (kbhit()) {  // This would need to be properly implemented
                    c = getchar();
                }
                cpu.a = (c == -1) ? 0 : c;
            }
            break;
            
        default:
            printf("Unimplemented KERNAL routine at $%04X\n", address);
            break;
    }
    
    // Emulate an RTS instruction
    cpu.pc = cpu_pull_word() + 1;
}

/**
 * Execute a number of CPU cycles
 */
void cpu_execute(uint32_t num_cycles) {
    uint32_t target_cycles = cycles + num_cycles;
    
    while (cycles < target_cycles) {
        cpu_step();
    }
}

/**
 * Print the current CPU state for debugging
 */
void cpu_print_state() {
    printf("CPU State:\n");
    printf("A: $%02X X: $%02X Y: $%02X SP: $%02X PC: $%04X\n", 
           cpu.a, cpu.x, cpu.y, cpu.sp, cpu.pc);
    printf("Flags: %c%c%c%c%c%c%c\n",
           cpu.n ? 'N' : '.', 
           cpu.v ? 'V' : '.', 
           cpu.b ? 'B' : '.', 
           cpu.d ? 'D' : '.', 
           cpu.i ? 'I' : '.', 
           cpu.z ? 'Z' : '.', 
           cpu.c ? 'C' : '.');
}

/**
 * Set the CPU program counter
 */
void cpu_set_pc(uint16_t address) {
    cpu.pc = address;
}