CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lncurses

SOURCES = chroma-0.1.c
OBJECTS = $(SOURCES:.c=.o)

# Default target
all: chroma-0.1

# Link object files to create the executable
chroma-0.1: $(OBJECTS)
	$(CC) -o chroma-0.1 $(OBJECTS) $(LDFLAGS)

# Compile source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# Clean up generated files
clean:
	rm -f chroma-0.1 $(OBJECTS)
