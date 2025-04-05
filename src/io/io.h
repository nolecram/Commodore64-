/**
 * io.h
 * I/O operations for the Commodore 64 emulator
 */

#ifndef IO_H
#define IO_H

#include <stdint.h>

// VIC-II Video Interface Controller
#define VIC_BASE_ADDRESS 0xD000
#define VIC_REGISTERS_SIZE 0x40

// SID Sound Interface Device
#define SID_BASE_ADDRESS 0xD400
#define SID_REGISTERS_SIZE 0x20

// CIA Complex Interface Adapter
#define CIA1_BASE_ADDRESS 0xDC00
#define CIA2_BASE_ADDRESS 0xDD00
#define CIA_REGISTERS_SIZE 0x10

// I/O functions
void io_init();
void io_update();
uint8_t io_read(uint16_t address);
void io_write(uint16_t address, uint8_t value);
void io_handle_keyboard_input();
void io_set_key_pressed(uint8_t key, int is_pressed);

// Screen functions
void io_clear_screen();
void io_print_text(uint8_t x, uint8_t y, const char* text);
void io_update_display();

// Audio functions
void io_beep();
void io_set_audio_enabled(int enabled);

#endif /* IO_H */