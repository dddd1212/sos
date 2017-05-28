CC = x86_64-elf-gcc
AS = x86_64-elf-as
LD = x86_64-elf-ld
OUT_DIR = ../_output/system
qkr_sync:
	$(CC) -c *.c -Wall -fpic -fvisibility=hidden -g -o qkr_sync.o
	$(LD) -fpic -fvisibility=hidden -shared qkr_sync.o -o $(OUT_DIR)/qkr_sync.qkr

all: qkr_sync

clean:
	rm -f qkr_sync.o
	rm -f qkr_sync.qmo
