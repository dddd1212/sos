CC = x86_64-elf-gcc
AS = x86_64-elf-as
LD = x86_64-elf-ld
OUT_DIR = ../_output/system
disk_manager:
	$(CC) -c *.c -Wall -fpic -fvisibility=hidden -g -o disk_manager.o
	$(LD) -fpic -fvisibility=hidden -shared disk_manager.o -o $(OUT_DIR)/disk_manager.qkr

all: disk_manager

clean:
	rm -f disk_manager.o
	rm -f disk_manager.qmo
