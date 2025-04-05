/**
 * shell.c
 * Shell interface implementation for the Commodore 64 emulator
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "shell.h"
#include "../cpu/cpu.h"
#include "../memory/memory.h"
#include "../io/io.h"

// Shell state
static int running = 0;
static int in_basic_mode = 0;
static char input_buffer[256];

/**
 * Initialize the shell
 */
void shell_init() {
    running = 1;
    in_basic_mode = 0;
    
    printf("Commodore 64 Emulator Shell\n");
    printf("Type 'help' for a list of commands\n");
}

/**
 * Run the shell main loop
 */
void shell_run() {
    while (running) {
        shell_prompt();
        shell_handle_input();
    }
}

/**
 * Display the shell prompt
 */
void shell_prompt() {
    if (in_basic_mode) {
        printf("READY.\n");
    }
    printf("> ");
    fflush(stdout);
}

/**
 * Handle user input from the shell
 */
void shell_handle_input() {
    if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) {
        // Handle EOF
        running = 0;
        return;
    }
    
    // Remove trailing newline
    size_t len = strlen(input_buffer);
    if (len > 0 && input_buffer[len - 1] == '\n') {
        input_buffer[len - 1] = '\0';
    }
    
    // Skip empty lines
    if (strlen(input_buffer) == 0) {
        return;
    }
    
    // Process the input
    if (in_basic_mode) {
        shell_process_basic_line(input_buffer);
    } else {
        // Split into command and arguments
        char* args = input_buffer;
        while (*args && !isspace(*args)) {
            args++;
        }
        if (*args) {
            *args = '\0';
            args++;
            // Skip leading whitespace in args
            while (*args && isspace(*args)) {
                args++;
            }
        }
        
        // Parse and execute the command
        ShellCommand cmd = shell_parse_command(input_buffer);
        shell_execute_command(cmd, args);
    }
}

/**
 * Parse a command string into a ShellCommand enum
 */
ShellCommand shell_parse_command(const char* input) {
    if (strcmp(input, "help") == 0) return CMD_HELP;
    if (strcmp(input, "run") == 0) return CMD_RUN;
    if (strcmp(input, "load") == 0) return CMD_LOAD;
    if (strcmp(input, "list") == 0) return CMD_LIST;
    if (strcmp(input, "dump") == 0) return CMD_DUMP;
    if (strcmp(input, "reset") == 0) return CMD_RESET;
    if (strcmp(input, "step") == 0) return CMD_STEP;
    if (strcmp(input, "trace") == 0) return CMD_TRACE;
    if (strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0) return CMD_QUIT;
    if (strcmp(input, "basic") == 0) return CMD_BASIC;
    if (strcmp(input, "poke") == 0) return CMD_POKE;
    if (strcmp(input, "peek") == 0) return CMD_PEEK;
    if (strcmp(input, "sys") == 0) return CMD_SYS;
    
    return CMD_UNKNOWN;
}

/**
 * Execute a shell command
 */
void shell_execute_command(ShellCommand cmd, const char* args) {
    switch (cmd) {
        case CMD_HELP:
            shell_print_help();
            break;
            
        case CMD_RUN:
            printf("Running program...\n");
            cpu_execute(1000000);  // Run for a large number of cycles
            break;
            
        case CMD_LOAD:
            if (!args || !*args) {
                printf("Usage: load <filename> [address]\n");
                printf("If address is not specified, the default is $0800\n");
            } else {
                char filename[256];
                uint16_t address = 0x0800;  // Default load address
                
                // Parse the arguments
                if (sscanf(args, "%255s %hx", filename, &address) < 1) {
                    printf("Error: Invalid arguments\n");
                    break;
                }
                
                printf("Loading program from '%s' to address $%04X...\n", filename, address);
                if (shell_load_file(filename, address)) {
                    printf("Program loaded successfully\n");
                } else {
                    printf("Failed to load program\n");
                }
            }
            break;
            
        case CMD_LIST:
            printf("Listing program...\n");
            // This would list the currently loaded BASIC program
            printf("Not implemented yet\n");
            break;
            
        case CMD_DUMP:
            {
                uint16_t start = 0;
                uint16_t length = 256;
                if (args && *args) {
                    sscanf(args, "%hx %hx", &start, &length);
                }
                memory_dump(start, length);
            }
            break;
            
        case CMD_RESET:
            printf("Resetting system...\n");
            cpu_reset();
            io_init();
            break;
            
        case CMD_STEP:
            {
                int count = 1;
                if (args && *args) {
                    count = atoi(args);
                }
                printf("Stepping %d instruction(s)...\n", count);
                for (int i = 0; i < count; i++) {
                    cpu_step();
                }
                cpu_print_state();
            }
            break;
            
        case CMD_TRACE:
            {
                int enabled = 1;
                if (args && *args) {
                    enabled = atoi(args);
                }
                printf("Trace mode %s\n", enabled ? "enabled" : "disabled");
                // This would enable/disable instruction tracing
                printf("Not implemented yet\n");
            }
            break;
            
        case CMD_QUIT:
            printf("Exiting emulator...\n");
            running = 0;
            break;
            
        case CMD_BASIC:
            printf("Entering BASIC mode\n");
            shell_enter_basic_mode();
            break;
            
        case CMD_POKE:
            {
                uint16_t address;
                uint8_t value;
                if (args && *args && sscanf(args, "%hu,%hhu", &address, &value) == 2) {
                    memory_write(address, value);
                    printf("Poked %d into address %d\n", value, address);
                } else {
                    printf("Usage: poke <address>,<value>\n");
                }
            }
            break;
            
        case CMD_PEEK:
            {
                uint16_t address;
                if (args && *args && sscanf(args, "%hu", &address) == 1) {
                    uint8_t value = memory_read(address);
                    printf("Peek(%d) = %d ($%02X)\n", address, value, value);
                } else {
                    printf("Usage: peek <address>\n");
                }
            }
            break;
            
        case CMD_SYS:
            {
                uint16_t address;
                if (args && *args && sscanf(args, "%hx", &address) == 1) {
                    printf("Calling system routine at $%04X...\n", address);
                    // Set the PC to the specified address using the API
                    cpu_set_pc(address);
                    // Execute a number of instructions
                    cpu_execute(1000000);  // Run for many cycles
                    cpu_print_state();
                } else {
                    printf("Usage: sys <address>\n");
                }
            }
            break;
            
        case CMD_UNKNOWN:
        default:
            printf("Unknown command: %s\n", input_buffer);
            printf("Type 'help' for a list of commands\n");
            break;
    }
}

/**
 * Print help information
 */
void shell_print_help() {
    printf("Available commands:\n");
    printf("  help        - Show this help message\n");
    printf("  run         - Run the current program\n");
    printf("  load <file> - Load a program from a file\n");
    printf("  list        - List the current BASIC program\n");
    printf("  dump [addr] [len] - Dump memory contents\n");
    printf("  reset       - Reset the system\n");
    printf("  step [n]    - Execute n instructions (default: 1)\n");
    printf("  trace [0|1] - Enable/disable instruction tracing\n");
    printf("  basic       - Enter BASIC mode\n");
    printf("  poke a,v    - Write a value to memory address\n");
    printf("  peek a      - Read a value from memory address\n");
    printf("  sys addr    - Call a machine language routine\n");
    printf("  quit        - Exit the emulator\n");
}

/**
 * Enter BASIC mode
 */
void shell_enter_basic_mode() {
    in_basic_mode = 1;
    io_clear_screen();
    io_print_text(0, 0, "    **** COMMODORE 64 BASIC V2 ****");
    io_print_text(0, 2, " 64K RAM SYSTEM  38911 BASIC BYTES FREE");
    io_update_display();
}

/**
 * Exit BASIC mode
 */
void shell_exit_basic_mode() {
    in_basic_mode = 0;
}

/**
 * Check if in BASIC mode
 */
int shell_is_in_basic_mode() {
    return in_basic_mode;
}

/**
 * Process a line of BASIC input
 */
void shell_process_basic_line(const char* line) {
    // Exit BASIC mode if the user types "exit" or "quit"
    if (strcmp(line, "exit") == 0 || strcmp(line, "quit") == 0) {
        shell_exit_basic_mode();
        return;
    }
    
    // Just echo the line for now
    printf("BASIC: %s\n", line);
    
    // In a real implementation, this would parse and execute BASIC commands
    if (strncmp(line, "PRINT", 5) == 0 || strncmp(line, "print", 5) == 0) {
        // Very simple implementation of PRINT
        const char* text = line + 5;
        while (*text && isspace(*text)) text++;
        printf("%s\n", text);
    } else if (strncmp(line, "CLS", 3) == 0 || strncmp(line, "cls", 3) == 0) {
        // Clear the screen
        io_clear_screen();
        io_update_display();
    } else if (strlen(line) > 0) {
        printf("?SYNTAX ERROR\n");
    }
}

/**
 * Load a binary program file into memory
 */
int shell_load_file(const char* filename, uint16_t load_address) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Error: Could not open file %s\n", filename);
        return 0;
    }
    
    // Get the file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Allocate buffer for the file data
    uint8_t* buffer = (uint8_t*)malloc(file_size);
    if (buffer == NULL) {
        printf("Error: Could not allocate memory for file data\n");
        fclose(file);
        return 0;
    }
    
    // Read the file data
    size_t read_size = fread(buffer, 1, file_size, file);
    fclose(file);
    
    if (read_size != file_size) {
        printf("Error: Could not read all data from file\n");
        free(buffer);
        return 0;
    }
    
    // Load the data into memory
    memory_load(load_address, buffer, file_size);
    free(buffer);
    
    printf("Loaded %ld bytes from '%s' into memory at $%04X\n", file_size, filename, load_address);
    return 1;
}