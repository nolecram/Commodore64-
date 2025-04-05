/**
 * memory.c
 * Memory management implementation for the Commodore 64 emulator
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"

// Main memory array
static uint8_t memory[MEMORY_SIZE];

// Memory banking control flags
static int basic_rom_enabled = 1;
static int kernal_rom_enabled = 1;
static int char_rom_enabled = 1;
static int io_enabled = 1;

// ROM Data (would be loaded from files in a full implementation)
static uint8_t basic_rom[8192];   // 8K BASIC ROM
static uint8_t kernal_rom[8192];  // 8K KERNAL ROM
static uint8_t char_rom[4096];    // 4K Character ROM

// Memory access cache for faster lookups
static uint8_t *memory_read_map[256];  // Fast lookup for pages (256 pages of 256 bytes)

/**
 * Update memory banking and read/write maps
 */
static void update_memory_maps() {
    // Set up default read map (points to RAM)
    for (int i = 0; i < 256; i++) {
        memory_read_map[i] = &memory[i << 8];
    }
    
    // Update maps based on banking configuration
    if (basic_rom_enabled) {
        // BASIC ROM from $A000-$BFFF
        for (int i = 0xA0; i <= 0xBF; i++) {
            memory_read_map[i] = &basic_rom[(i - 0xA0) << 8];
        }
    }
    
    if (kernal_rom_enabled) {
        // KERNAL ROM from $E000-$FFFF
        for (int i = 0xE0; i <= 0xFF; i++) {
            memory_read_map[i] = &kernal_rom[(i - 0xE0) << 8];
        }
    }
    
    if (io_enabled) {
        // I/O region at $D000-$DFFF
        // In a real implementation, this would be handled specially
    } else if (char_rom_enabled) {
        // Character ROM at $D000-$DFFF when I/O is disabled
        for (int i = 0xD0; i <= 0xDF; i++) {
            memory_read_map[i] = &char_rom[(i - 0xD0) << 8];
        }
    }
}

/**
 * Initialize the memory system
 */
void memory_init() {
    // Clear all memory
    memset(memory, 0, MEMORY_SIZE);
    
    // Initialize ROM areas with placeholder patterns
    memset(basic_rom, 0xEA, sizeof(basic_rom));    // NOP instructions
    memset(kernal_rom, 0xEA, sizeof(kernal_rom));  // NOP instructions
    memset(char_rom, 0x00, sizeof(char_rom));      // Empty characters
    
    // Set up default memory configuration
    basic_rom_enabled = 1;
    kernal_rom_enabled = 1;
    char_rom_enabled = 1;
    io_enabled = 1;
    
    // Initialize memory with some default values
    memory[0x0000] = 0x2F;  // Default data direction register
    memory[0x0001] = 0x37;  // Default processor port
    
    // Set up reset vectors
    kernal_rom[0x1FFC - 0xE000] = 0x00;  // Set reset vector to $E000
    kernal_rom[0x1FFD - 0xE000] = 0xE0;
    
    // Set up interrupt vectors
    kernal_rom[0x1FFA - 0xE000] = 0x43;  // NMI vector
    kernal_rom[0x1FFB - 0xE000] = 0xFE;
    kernal_rom[0x1FFE - 0xE000] = 0x48;  // IRQ/BRK vector
    kernal_rom[0x1FFF - 0xE000] = 0xFF;
    
    // Initialize memory maps
    update_memory_maps();
}

/**
 * Read a byte from memory, taking into account memory banking
 */
uint8_t memory_read(uint16_t address) {
    uint8_t page = address >> 8;
    uint8_t offset = address & 0xFF;
    
    // Special handling for I/O region
    if (page >= 0xD0 && page <= 0xDF && io_enabled) {
        // In a full implementation, this would handle I/O chip access
        return memory[address];
    }
    
    // Use the memory map for fast lookups
    return memory_read_map[page][offset];
}

/**
 * Write a byte to memory, taking into account memory banking
 */
void memory_write(uint16_t address, uint8_t value) {
    // Check if writing to ROM regions (writes are ignored in ROM)
    if (basic_rom_enabled && address >= BASIC_ROM_START && address <= BASIC_ROM_END) {
        // Writing to BASIC ROM area is ignored when ROM is enabled
        return;
    } else if (kernal_rom_enabled && address >= KERNAL_ROM_START && address <= KERNAL_ROM_END) {
        // Writing to KERNAL ROM area is ignored when ROM is enabled
        return;
    } else if (address >= IO_REGION_START && address <= IO_REGION_END) {
        if (io_enabled) {
            // Writing to I/O region
            // In a full implementation, this would handle I/O chip access
            memory[address] = value;
            return;
        } else if (char_rom_enabled) {
            // Writing to Character ROM is ignored
            return;
        }
    }
    
    // Special handling for memory control register at $01
    if (address == 0x0001) {
        // Bits 0-2 control memory banking
        uint8_t old_value = memory[0x0001];
        memory[0x0001] = value;
        
        kernal_rom_enabled = (value & 0x02) != 0;
        basic_rom_enabled = (value & 0x03) != 0;
        io_enabled = (value & 0x04) != 0;
        char_rom_enabled = (value & 0x04) == 0 && (value & 0x03) != 0;
        
        // Only update maps if banking configuration changed
        if ((old_value & 0x07) != (value & 0x07)) {
            update_memory_maps();
        }
        return;
    }
    
    // Default case: write to RAM
    memory[address] = value;
}

/**
 * Load data into memory
 */
void memory_load(uint16_t address, uint8_t *data, uint16_t length) {
    // Check if the data will fit in memory
    if (address + length > MEMORY_SIZE) {
        printf("Warning: Data exceeds memory bounds\n");
        length = MEMORY_SIZE - address;
    }
    
    // Copy the data into memory
    memcpy(&memory[address], data, length);
}

/**
 * Load ROM data from a file
 */
int memory_load_rom(const char *filename, uint8_t *rom_buffer, size_t rom_size) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Error: Could not open ROM file: %s\n", filename);
        return 0;
    }
    
    size_t bytes_read = fread(rom_buffer, 1, rom_size, file);
    fclose(file);  // Fixed missing parenthesis
    
    if (bytes_read != rom_size) {
        printf("Warning: ROM file size mismatch (%zu bytes read, expected %zu)\n", 
               bytes_read, rom_size);
    }
    
    // Update memory maps after loading ROM
    update_memory_maps();
    
    return 1;
}

/**
 * Load BASIC ROM from a file
 */
int memory_load_basic_rom(const char *filename) {
    return memory_load_rom(filename, basic_rom, sizeof(basic_rom));
}

/**
 * Load KERNAL ROM from a file
 */
int memory_load_kernal_rom(const char *filename) {
    return memory_load_rom(filename, kernal_rom, sizeof(kernal_rom));
}

/**
 * Load Character ROM from a file
 */
int memory_load_char_rom(const char *filename) {
    return memory_load_rom(filename, char_rom, sizeof(char_rom));
}

/**
 * Dump memory contents for debugging
 */
void memory_dump(uint16_t start_address, uint16_t length) {
    uint16_t end_address = start_address + length - 1;
    if (end_address >= MEMORY_SIZE) {
        end_address = MEMORY_SIZE - 1;
    }
    
    printf("Memory dump from $%04X to $%04X:\n", start_address, end_address);
    
    for (uint16_t addr = start_address; addr <= end_address; addr++) {
        if ((addr - start_address) % 16 == 0) {
            printf("\n$%04X: ", addr);
        }
        printf("%02X ", memory_read(addr));  // Use memory_read to show ROM contents
    }
    printf("\n");
}