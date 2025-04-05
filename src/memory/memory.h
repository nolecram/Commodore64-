/**
 * memory.h - Memory management for the Commodore 64 emulator
 * 
 * This module implements the memory management system for the Commodore 64,
 * which features a complex memory banking arrangement with multiple
 * overlapping regions of RAM, ROM, and I/O.
 * 
 * The C64 has 64KB of addressable memory space, but actually contains more
 * than 64KB of physical memory when considering all ROM and I/O regions.
 * The processor port at address $01 controls which memory banks are active.
 * 
 * Memory Map Overview:
 * - $0000-$00FF: Zero Page (fast access memory for the CPU)
 * - $0100-$01FF: Stack (used by the CPU for subroutine calls and interrupts)
 * - $0200-$03FF: System variables and buffers
 * - $0400-$07FF: Screen memory (1000 bytes for 40x25 character display)
 * - $0800-$9FFF: BASIC program space (38K of free RAM for BASIC)
 * - $A000-$BFFF: BASIC ROM or RAM, depending on banking
 * - $C000-$CFFF: Free RAM (when BASIC ROM is banked out)
 * - $D000-$DFFF: I/O devices or Character ROM or RAM, depending on banking
 * - $E000-$FFFF: KERNAL ROM or RAM, depending on banking
 * 
 * @copyright This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>

/**
 * Total addressable memory size
 * The Commodore 64 has a 16-bit address bus, allowing for 64KB of addressable memory
 */
#define MEMORY_SIZE 65536

/**
 * Initialize the memory system
 * Sets up RAM, ROM regions, and initial memory configuration
 */
void memory_init();

/**
 * Read a byte from memory
 * Takes into account the current memory banking configuration
 * 
 * @param address 16-bit memory address to read from
 * @return The byte value at the specified address
 */
uint8_t memory_read(uint16_t address);

/**
 * Write a byte to memory
 * Takes into account the current memory banking configuration
 * Writing to ROM areas is ignored when those ROMs are enabled
 * 
 * @param address 16-bit memory address to write to
 * @param value The byte value to write
 */
void memory_write(uint16_t address, uint8_t value);

/**
 * Load data into memory
 * Copies a block of data into memory starting at the specified address
 * 
 * @param address 16-bit starting address for the data
 * @param data Pointer to the source data
 * @param length Number of bytes to copy
 */
void memory_load(uint16_t address, uint8_t *data, uint16_t length);

/**
 * Dump memory contents
 * Displays a formatted hex dump of memory for debugging
 * 
 * @param start_address 16-bit starting address for the dump
 * @param length Number of bytes to dump
 */
void memory_dump(uint16_t start_address, uint16_t length);

/**
 * Load ROM data from a file
 * General function for loading any ROM file into a buffer
 * 
 * @param filename Path to the ROM file
 * @param rom_buffer Buffer to store the ROM data
 * @param rom_size Size of the buffer/expected ROM size
 * @return 1 on success, 0 on failure
 */
int memory_load_rom(const char *filename, uint8_t *rom_buffer, size_t rom_size);

/**
 * Load BASIC ROM from a file
 * Loads the 8KB BASIC interpreter ROM
 * 
 * @param filename Path to the BASIC ROM file
 * @return 1 on success, 0 on failure
 */
int memory_load_basic_rom(const char *filename);

/**
 * Load KERNAL ROM from a file
 * Loads the 8KB KERNAL operating system ROM
 * 
 * @param filename Path to the KERNAL ROM file
 * @return 1 on success, 0 on failure
 */
int memory_load_kernal_rom(const char *filename);

/**
 * Load Character ROM from a file
 * Loads the 4KB character generator ROM
 * 
 * @param filename Path to the Character ROM file
 * @return 1 on success, 0 on failure
 */
int memory_load_char_rom(const char *filename);

/**
 * Memory Region Definitions
 * These define the boundaries of special memory areas in the C64
 */
#define BASIC_ROM_START       0xA000  // Start of BASIC ROM (8KB)
#define BASIC_ROM_END         0xBFFF  // End of BASIC ROM
#define KERNAL_ROM_START      0xE000  // Start of KERNAL ROM (8KB)
#define KERNAL_ROM_END        0xFFFF  // End of KERNAL ROM
#define CHAR_ROM_START        0xD000  // Start of Character ROM (4KB)
#define CHAR_ROM_END          0xDFFF  // End of Character ROM
#define IO_REGION_START       0xD000  // Start of I/O region (overlaps with CHAR ROM)
#define IO_REGION_END         0xDFFF  // End of I/O region
#define COLOR_RAM_START       0xD800  // Start of Color RAM (1KB)
#define COLOR_RAM_END         0xDBFF  // End of Color RAM
#define SCREEN_MEMORY_START   0x0400  // Start of screen memory (1KB)
#define SCREEN_MEMORY_END     0x07FF  // End of screen memory

#endif /* MEMORY_H */