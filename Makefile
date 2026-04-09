.PHONY = all clean

CC = gcc
CFLAGS = -Wall
LDFLAGS = -lm

COMMON = huffman_tree.o input_output.o queue.o
COMPRESSOR = compressor.o $(COMMON)
DECOMPRESSOR = decompressor.o $(COMMON)

all: jam unjam softclean

jam: $(COMPRESSOR)
	@echo "Compiling compressor..."
	@$(CC) $^ -o $@ $(LDFLAGS)

unjam: $(DECOMPRESSOR)
	@echo "Compiling decompressor..."
	@$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $@

softclean:
	@rm -f *.o

clean:
	@echo "Cleaning up..."
	@rm -f *.o jam unjam
