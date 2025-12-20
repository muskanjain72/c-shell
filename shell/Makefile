# CC = gcc
# CFLAGS = -std=c99 -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700 -Wall -Wextra -Werror -Wno-unused-parameter -fno-asm

# SRCS = main.c prompt.c input.c parser.c hop.c reveal.c log.c pipes.c activities.c ping.c signals.c jobs.c
# OBJS = $(patsubst %.c, %.o, $(SRCS))

# EXECUTABLE = shell.out

# .PHONY: all clean

# all: $(EXECUTABLE)

# $(EXECUTABLE): $(OBJS)
# 	$(CC) $(CFLAGS) $(OBJS) -o $@

# %.o: %.c
# 	$(CC) $(CFLAGS) -c $< -o $@

# clean:
# 	rm -f $(OBJS) $(EXECUTABLE)
# Compiler
CC = gcc
CFLAGS = -Wall -g -Iinclude   # <-- add include path here

# Source files (all .c files in src/)
SRCS = $(wildcard src/*.c)

# Object files (put them in src/ as well)
OBJS = $(SRCS:.c=.o)

# Executable
TARGET = shell.out

# Default target
all: $(TARGET)

# Link objects to create executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Compile .c to .o
src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(OBJS) $(TARGET)
