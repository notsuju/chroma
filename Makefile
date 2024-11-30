CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lncurses

SOURCES = chroma.c
OBJECTS = $(SOURCES:.c=.o)

# Default target
all: chroma

# Link object files to create the executable
chroma: $(OBJECTS)
	$(CC) -o chroma $(OBJECTS) $(LDFLAGS)

# Compile source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# Clean up generated files
clean:
	rm -f chroma $(OBJECTS)
