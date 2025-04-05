/**
 * hello_world.c
 * A simple hello world program for the Commodore 64 emulator
 * 
 * This program will load into memory and display "HELLO, COMMODORE 64!"
 * on the screen when executed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Simple 6502/6510 machine code "Hello World" program
static uint8_t hello_world_program[] = {
    // Initialize
    0xA9, 0x93,       // LDA #$93 (Clear screen code in PETSCII)
    0x20, 0xD2, 0xFF, // JSR $FFD2 (KERNAL routine for printing a character)
    
    // Print H
    0xA9, 0x48,       // LDA #$48 ('H')
    0x20, 0xD2, 0xFF, // JSR $FFD2
    
    // Print E
    0xA9, 0x45,       // LDA #$45 ('E')
    0x20, 0xD2, 0xFF, // JSR $FFD2
    
    // Print L
    0xA9, 0x4C,       // LDA #$4C ('L')
    0x20, 0xD2, 0xFF, // JSR $FFD2
    0xA9, 0x4C,       // LDA #$4C ('L')
    0x20, 0xD2, 0xFF, // JSR $FFD2
    
    // Print O
    0xA9, 0x4F,       // LDA #$4F ('O')
    0x20, 0xD2, 0xFF, // JSR $FFD2
    
    // Print ,
    0xA9, 0x2C,       // LDA #$2C (',')
    0x20, 0xD2, 0xFF, // JSR $FFD2
    
    // Print space
    0xA9, 0x20,       // LDA #$20 (' ')
    0x20, 0xD2, 0xFF, // JSR $FFD2
    
    // Print C
    0xA9, 0x43,       // LDA #$43 ('C')
    0x20, 0xD2, 0xFF, // JSR $FFD2
    
    // Print O
    0xA9, 0x4F,       // LDA #$4F ('O')
    0x20, 0xD2, 0xFF, // JSR $FFD2
    
    // Print M
    0xA9, 0x4D,       // LDA #$4D ('M')
    0x20, 0xD2, 0xFF, // JSR $FFD2
    
    // Print M
    0xA9, 0x4D,       // LDA #$4D ('M')
    0x20, 0xD2, 0xFF, // JSR $FFD2
    
    // Print O
    0xA9, 0x4F,       // LDA #$4F ('O')
    0x20, 0xD2, 0xFF, // JSR $FFD2
    
    // Print D
    0xA9, 0x44,       // LDA #$44 ('D')
    0x20, 0xD2, 0xFF, // JSR $FFD2
    
    // Print O
    0xA9, 0x4F,       // LDA #$4F ('O')
    0x20, 0xD2, 0xFF, // JSR $FFD2
    
    // Print R
    0xA9, 0x52,       // LDA #$52 ('R')
    0x20, 0xD2, 0xFF, // JSR $FFD2
    
    // Print E
    0xA9, 0x45,       // LDA #$45 ('E')
    0x20, 0xD2, 0xFF, // JSR $FFD2
    
    // Print space
    0xA9, 0x20,       // LDA #$20 (' ')
    0x20, 0xD2, 0xFF, // JSR $FFD2
    
    // Print 6
    0xA9, 0x36,       // LDA #$36 ('6')
    0x20, 0xD2, 0xFF, // JSR $FFD2
    
    // Print 4
    0xA9, 0x34,       // LDA #$34 ('4')
    0x20, 0xD2, 0xFF, // JSR $FFD2
    
    // Print !
    0xA9, 0x21,       // LDA #$21 ('!')
    0x20, 0xD2, 0xFF, // JSR $FFD2
    
    // Infinite loop
    0x4C, 0x00, 0x08  // JMP $0800 (Loop back to the start)
};

/**
 * Utility function to write the program to a binary file
 */
int write_program_to_file(const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (file == NULL) {
        printf("Error: Could not open file %s for writing\n", filename);
        return 0;
    }
    
    // Write the binary data to the file
    size_t written = fwrite(hello_world_program, 1, sizeof(hello_world_program), file);
    fclose(file);
    
    if (written != sizeof(hello_world_program)) {
        printf("Error: Could not write all data to file\n");
        return 0;
    }
    
    printf("Successfully wrote program to %s\n", filename);
    return 1;
}

/**
 * Main function
 */
int main(int argc, char* argv[]) {
    const char* default_filename = "hello_world.prg";
    const char* filename = (argc > 1) ? argv[1] : default_filename;
    
    printf("Hello World Demo Program Generator for Commodore 64 Emulator\n");
    printf("This program will create a binary file containing a simple\n");
    printf("machine code program that displays 'HELLO, COMMODORE 64!' on the screen.\n\n");
    
    if (write_program_to_file(filename)) {
        printf("Demo program created successfully in %s\n", filename);
        printf("Load this program into the emulator at address $0800 and run it.\n");
    } else {
        printf("Failed to create demo program.\n");
        return 1;
    }
    
    return 0;
}