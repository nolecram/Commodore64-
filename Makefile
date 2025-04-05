# Makefile for Commodore 64 Emulator

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g -I.

# Source files
SRC = src/main.c \
      src/cpu/cpu.c \
      src/memory/memory.c \
      src/io/io.c \
      src/shell/shell.c

# Object files
OBJ = $(SRC:.c=.o)

# Binary name
TARGET = c64emu

# Default target
all: $(TARGET)

# Link the object files to create the binary
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean up
clean:
	rm -f $(OBJ) $(TARGET)

# Clean and rebuild
rebuild: clean all

# Run the emulator
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean rebuild run