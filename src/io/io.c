/**
 * io.c
 * I/O operations implementation for the Commodore 64 emulator
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "io.h"
#include "../memory/memory.h"

// I/O registers
static uint8_t vic_registers[VIC_REGISTERS_SIZE];
static uint8_t sid_registers[SID_REGISTERS_SIZE];
static uint8_t cia1_registers[CIA_REGISTERS_SIZE];
static uint8_t cia2_registers[CIA_REGISTERS_SIZE];

// Screen data
static uint8_t screen_data[40 * 25];
static uint8_t color_data[40 * 25];

// Keyboard state
static uint8_t keyboard_matrix[8];  // 8x8 keyboard matrix
static int audio_enabled = 1;

/**
 * Initialize the I/O subsystems
 */
void io_init() {
    // Clear all registers
    memset(vic_registers, 0, sizeof(vic_registers));
    memset(sid_registers, 0, sizeof(sid_registers));
    memset(cia1_registers, 0, sizeof(cia1_registers));
    memset(cia2_registers, 0, sizeof(cia2_registers));
    
    // Clear screen and color memory
    memset(screen_data, 32, sizeof(screen_data));  // Fill with spaces
    memset(color_data, 14, sizeof(color_data));    // Fill with light blue
    
    // Clear keyboard matrix
    memset(keyboard_matrix, 0xFF, sizeof(keyboard_matrix));
    
    // Set up default VIC-II registers
    vic_registers[0x11] = 0x1B;  // Screen control register
    vic_registers[0x16] = 0x08;  // Screen control register
    vic_registers[0x18] = 0x14;  // Memory setup
    vic_registers[0x20] = 0x0F;  // Border color (light blue)
    vic_registers[0x21] = 0x06;  // Background color (blue)
    
    // Set up default CIA registers
    cia1_registers[0x0D] = 0x00;  // CIA 1 ICR
    cia2_registers[0x0D] = 0x00;  // CIA 2 ICR
    
    // Clear the screen
    io_clear_screen();
}

/**
 * Update I/O state (called once per frame)
 */
void io_update() {
    // Update timers and other I/O components that need regular updates
    // For now, this is a placeholder
    
    // Update the display
    io_update_display();
}

/**
 * Read from an I/O register
 */
uint8_t io_read(uint16_t address) {
    // VIC-II registers (Video Interface Controller)
    if (address >= VIC_BASE_ADDRESS && address < VIC_BASE_ADDRESS + VIC_REGISTERS_SIZE) {
        return vic_registers[address - VIC_BASE_ADDRESS];
    }
    
    // SID registers (Sound Interface Device)
    if (address >= SID_BASE_ADDRESS && address < SID_BASE_ADDRESS + SID_REGISTERS_SIZE) {
        return sid_registers[address - SID_BASE_ADDRESS];
    }
    
    // CIA1 registers (Complex Interface Adapter 1 - Keyboard, Joystick, etc.)
    if (address >= CIA1_BASE_ADDRESS && address < CIA1_BASE_ADDRESS + CIA_REGISTERS_SIZE) {
        // Special handling for keyboard matrix
        if (address == CIA1_BASE_ADDRESS) {
            // Keyboard matrix
            uint8_t row_select = cia1_registers[0] & 0xFF;
            uint8_t result = 0xFF;
            
            // Return key state for the selected rows
            for (int row = 0; row < 8; row++) {
                if (!(row_select & (1 << row))) {
                    result &= keyboard_matrix[row];
                }
            }
            return result;
        }
        return cia1_registers[address - CIA1_BASE_ADDRESS];
    }
    
    // CIA2 registers (Complex Interface Adapter 2 - Serial bus, User port, etc.)
    if (address >= CIA2_BASE_ADDRESS && address < CIA2_BASE_ADDRESS + CIA_REGISTERS_SIZE) {
        return cia2_registers[address - CIA2_BASE_ADDRESS];
    }
    
    // Default case - return 0xFF for unimplemented registers
    return 0xFF;
}

/**
 * Write to an I/O register
 */
void io_write(uint16_t address, uint8_t value) {
    // VIC-II registers
    if (address >= VIC_BASE_ADDRESS && address < VIC_BASE_ADDRESS + VIC_REGISTERS_SIZE) {
        vic_registers[address - VIC_BASE_ADDRESS] = value;
        return;
    }
    
    // SID registers
    if (address >= SID_BASE_ADDRESS && address < SID_BASE_ADDRESS + SID_REGISTERS_SIZE) {
        sid_registers[address - SID_BASE_ADDRESS] = value;
        // If writing to a frequency register, potentially generate sound
        if ((address - SID_BASE_ADDRESS) == 0x01 && audio_enabled) {
            io_beep();
        }
        return;
    }
    
    // CIA1 registers
    if (address >= CIA1_BASE_ADDRESS && address < CIA1_BASE_ADDRESS + CIA_REGISTERS_SIZE) {
        cia1_registers[address - CIA1_BASE_ADDRESS] = value;
        return;
    }
    
    // CIA2 registers
    if (address >= CIA2_BASE_ADDRESS && address < CIA2_BASE_ADDRESS + CIA_REGISTERS_SIZE) {
        cia2_registers[address - CIA2_BASE_ADDRESS] = value;
        return;
    }
}

/**
 * Handle keyboard input
 */
void io_handle_keyboard_input() {
    // In a real implementation, this would read input from the host system
    // and update the keyboard matrix accordingly
    // For now, this is a placeholder
}

/**
 * Set a key in the keyboard matrix as pressed or released
 */
void io_set_key_pressed(uint8_t key, int is_pressed) {
    // Key is encoded as (row << 4) | column
    uint8_t row = (key >> 4) & 0x07;
    uint8_t col = key & 0x07;
    
    if (is_pressed) {
        // When a key is pressed, the corresponding bit is cleared
        keyboard_matrix[row] &= ~(1 << col);
    } else {
        // When a key is released, the corresponding bit is set
        keyboard_matrix[row] |= (1 << col);
    }
}

/**
 * Clear the screen
 */
void io_clear_screen() {
    memset(screen_data, 32, sizeof(screen_data));  // Fill with spaces
    
    // Update screen memory
    for (int i = 0; i < 1000; i++) {
        memory_write(SCREEN_MEMORY_START + i, 32);  // Space character
        memory_write(COLOR_RAM_START + i, 14);     // Light blue
    }
}

/**
 * Print text at the specified screen position
 */
void io_print_text(uint8_t x, uint8_t y, const char* text) {
    if (x >= 40 || y >= 25) {
        return;  // Out of bounds
    }
    
    uint16_t screen_pos = y * 40 + x;
    uint16_t text_len = strlen(text);
    
    // Make sure we don't print beyond the screen boundaries
    if (screen_pos + text_len > 1000) {
        text_len = 1000 - screen_pos;
    }
    
    // Copy text to screen
    for (uint16_t i = 0; i < text_len; i++) {
        char c = text[i];
        
        // Convert character to C64 PETSCII character set
        uint8_t petscii;
        
        // Very simplified conversion, a real implementation would be more complex
        if (c >= 'a' && c <= 'z') {
            petscii = c - ('a' - 1);  // Lowercase letters
        } else if (c >= 'A' && c <= 'Z') {
            petscii = c - ('A' - 65);  // Uppercase letters
        } else if (c >= '0' && c <= '9') {
            petscii = c;  // Numbers
        } else if (c == ' ') {
            petscii = 32;  // Space
        } else {
            petscii = c;  // Other characters, not properly mapped
        }
        
        // Write to screen memory
        memory_write(SCREEN_MEMORY_START + screen_pos + i, petscii);
        // Also update our internal buffer
        screen_data[screen_pos + i] = petscii;
    }
}

/**
 * Update the display
 */
void io_update_display() {
    // In a real implementation, this would render the screen based on VIC-II state
    // For now, this is a simplified version that just uses our internal buffer
    
    // Clear terminal (system-dependent)
    printf("\033[2J\033[H");  // ANSI escape sequence to clear screen and move cursor to home
    
    // Draw the screen
    for (int y = 0; y < 25; y++) {
        for (int x = 0; x < 40; x++) {
            uint8_t character = screen_data[y * 40 + x];
            
            // Convert PETSCII to ASCII for terminal output (very simplified)
            char ascii;
            if (character >= 1 && character <= 26) {
                ascii = character + ('a' - 1);  // Convert back to lowercase
            } else if (character >= 65 && character <= 90) {
                ascii = character;  // Already uppercase
            } else if (character >= 193 && character <= 218) {
                ascii = character - (193 - 'a');  // Shifted lowercase
            } else if (character == 32) {
                ascii = ' ';  // Space
            } else {
                ascii = '.';  // Default for non-printable
            }
            
            printf("%c", ascii);
        }
        printf("\n");
    }
}

/**
 * Generate a beep sound
 */
void io_beep() {
    if (audio_enabled) {
        // In a real implementation, this would use the host system to generate sound
        // For now, just print a placeholder
        printf("\007");  // ASCII BEL character may generate a beep on some terminals
    }
}

/**
 * Enable or disable audio
 */
void io_set_audio_enabled(int enabled) {
    audio_enabled = enabled;
}