CC = x86_64-elf-gcc
AS = x86_64-elf-as
LD = x86_64-elf-ld
OUT_DIR = ../_output/system
scheduler:
	$(AS) scheduler_asm.asm -g -o scheduler_asm.o
	$(CC) -c *.c -Wall -fpic -fvisibility=hidden -g
	$(LD) -fpic -fvisibility=hidden -shared *.o -o $(OUT_DIR)/scheduler.qkr

all: scheduler

clean:
	rm -f scheduler.o
	rm -f scheduler.qmo
