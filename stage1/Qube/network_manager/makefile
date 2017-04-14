CC = x86_64-elf-gcc
AS = x86_64-elf-as
LD = x86_64-elf-ld
OUT_DIR = ../_output/system
network_manager:
	$(CC) -c *.c -fpic -fvisibility=hidden -g -o network_manager.o
	$(LD) -fpic -fvisibility=hidden -shared network_manager.o -o $(OUT_DIR)/network_manager.qkr

all: network_manager

clean:
	rm -f network_manager.o
	rm -f network_manager.qmo
