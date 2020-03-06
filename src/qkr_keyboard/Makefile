CC = x86_64-elf-gcc
AS = x86_64-elf-as
LD = x86_64-elf-ld
OUT_DIR = ../_output/system
keyboard:
	$(CC) -c *.c -Wall -fpic -fvisibility=hidden -g
	$(LD) -fpic -fvisibility=hidden -shared *.o -o $(OUT_DIR)/keyboard.qkr

all: keyboard

clean:
	rm -f *.o
	rm -f $(OUT_DIR)/keyboard.qkr
