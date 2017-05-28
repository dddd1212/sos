CC = x86_64-elf-gcc
AS = x86_64-elf-as
LD = x86_64-elf-ld
OUT_DIR = ../_output/system
qbject_manager:
	$(CC) -c *.c -Wall -fpic -fvisibility=hidden -g -o qbject_manager.o
	$(LD) -fpic -fvisibility=hidden -shared qbject_manager.o -o $(OUT_DIR)/qbject_manager.qkr

all: qbject_manager

clean:
	rm -f qbject_manager.o
	rm -f qbject_manager.qmo
