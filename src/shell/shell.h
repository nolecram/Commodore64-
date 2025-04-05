/**
 * shell.h
 * Shell interface for the Commodore 64 emulator
 */

#ifndef SHELL_H
#define SHELL_H

#include <stdint.h>

// Shell commands
typedef enum {
    CMD_HELP,
    CMD_RUN,
    CMD_LOAD,
    CMD_LIST,
    CMD_DUMP,
    CMD_RESET,
    CMD_STEP,
    CMD_TRACE,
    CMD_QUIT,
    CMD_BASIC,
    CMD_POKE,
    CMD_PEEK,
    CMD_SYS,
    CMD_UNKNOWN
} ShellCommand;

// Shell functions
void shell_init();
void shell_run();
ShellCommand shell_parse_command(const char* input);
void shell_execute_command(ShellCommand cmd, const char* args);
void shell_print_help();
void shell_handle_input();
void shell_prompt();

// File operations
int shell_load_file(const char* filename, uint16_t load_address);

// BASIC mode
void shell_enter_basic_mode();
void shell_exit_basic_mode();
int shell_is_in_basic_mode();
void shell_process_basic_line(const char* line);

#endif /* SHELL_H */