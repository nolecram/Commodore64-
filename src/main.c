/**
 * main.c - Main entry point for the Commodore 64 emulator
 * 
 * This file contains the main program entry point and initialization routines
 * for the Commodore 64 emulator. It coordinates the startup of all subsystems
 * (CPU, memory, I/O, shell) and manages the main program loop.
 * 
 * The emulator recreates the experience of using a Commodore 64 through a
 * command-line shell interface, with support for running BASIC programs and
 * machine language routines.
 * 
 * @copyright This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cpu/cpu.h"
#include "memory/memory.h"
#include "io/io.h"
#include "shell/shell.h"

/**
 * Path to ROM files
 * These would normally be provided with the emulator distribution.
 * The emulator will fall back to built-in placeholders if these files are not found.
 */
#define ROMS_PATH "roms/"
#define BASIC_ROM_FILE ROMS_PATH "basic.rom"     // 8KB BASIC ROM
#define KERNAL_ROM_FILE ROMS_PATH "kernal.rom"   // 8KB KERNAL ROM
#define CHAR_ROM_FILE ROMS_PATH "chargen.rom"    // 4KB Character ROM

/**
 * Simple program to load into memory at startup
 * This small machine language program initializes some key memory locations
 * and jumps to the BASIC ROM entry point.
 */
static uint8_t basic_rom[] = {
    0xA9, 0x01,       // LDA #$01   ; Load accumulator with 1
    0x85, 0x02,       // STA $02    ; Store in location $02 (BASIC init flag)
    0x4C, 0x00, 0xA0  // JMP $A000  ; Jump to BASIC ROM entry point
};

/**
 * Try to load ROM files from disk
 * Falls back to built-in placeholder ROMs if the files are not found
 */
void load_roms() {
    int basic_loaded = memory_load_basic_rom(BASIC_ROM_FILE);
    int kernal_loaded = memory_load_kernal_rom(KERNAL_ROM_FILE);
    int char_loaded = memory_load_char_rom(CHAR_ROM_FILE);
    
    if (!basic_loaded || !kernal_loaded || !char_loaded) {
        printf("Some ROM files could not be loaded, using built-in placeholders\n");
    }
}

/**
 * Initialize all emulator subsystems
 * This function sets up memory, loads ROMs, initializes the CPU and I/O,
 * loads a simple startup program, and prepares the system for execution.
 */
void init_emulator() {
    // Initialize subsystems in the correct order
    memory_init();
    
    // Try to load ROM files
    load_roms();
    
    // Initialize remaining subsystems
    cpu_init();
    io_init();
    shell_init();
    
    // Load a simple program to simulate BASIC ROM
    memory_load(0x0800, basic_rom, sizeof(basic_rom));
    
    // Set the reset vector to point to our program
    memory_write(0xFFFC, 0x00);
    memory_write(0xFFFD, 0x08);
    
    // Reset the CPU to start execution
    cpu_reset();
    
    printf("Commodore 64 Emulator initialized successfully.\n");
}

/**
 * Display emulator and system information
 * Prints a welcome message and basic info about the emulated system
 */
void show_system_info() {
    printf("================================================\n");
    printf("  Commodore 64 Emulator\n");
    printf("================================================\n");
    printf("  Memory:      64K RAM + 20K ROM\n");
    printf("  Processor:   MOS Technology 6510\n");
    printf("  Clock speed: ~1 MHz\n");
    printf("================================================\n");
    printf("Type 'help' to see available commands\n\n");
}

/**
 * Main program entry point
 * Initializes the emulator, displays system information, and runs the shell
 * 
 * @param argc Number of command line arguments
 * @param argv Array of command line argument strings
 * @return Program exit code
 */
int main(int argc, char *argv[]) {
    printf("Commodore 64 Emulator starting...\n");
    
    // Create ROMs directory if it doesn't exist
    system("mkdir -p roms");
    
    // Initialize the emulator
    init_emulator();
    
    // Show system information
    show_system_info();
    
    // Run the shell interface
    shell_run();
    
    printf("Emulator shutdown complete.\n");
    return 0;
}