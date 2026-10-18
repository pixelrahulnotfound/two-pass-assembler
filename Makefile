CC=gcc
CFLAGS=-std=c89 -pedantic -W -Wall -Wpointer-arith -Wwrite-strings -Wstrict-prototypes
all: asm emu
asm: asm.c
	$(CC) $(CFLAGS) asm.c -o asm
emu: emu.c
	$(CC) $(CFLAGS) emu.c -o emu
clean:
	rm -f asm emu out.bin out.lst
