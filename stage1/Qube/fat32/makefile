CC = x86_64-elf-gcc
AS = x86_64-elf-as
LD = x86_64-elf-ld
OUT_DIR = ../_output/system
fat32:
	$(CC) -c *.c -Wall -fpic -fvisibility=hidden -g -o fat32.o
	$(LD) -fpic -fvisibility=hidden -shared fat32.o -o $(OUT_DIR)/fat32.qkr

all: fat32

clean:
	rm -f fat32.o
	rm -f fat32.qmo
