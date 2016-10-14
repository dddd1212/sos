CC = x86_64-elf-gcc.exe
AS = x86_64-elf-as.exe
LD = x86_64-elf-ld.exe
OUT_DIR = ../_output/system
scheduler:
	$(AS) scheduler_asm.asm -g -o scheduler_asm.o
	$(CC) -c *.c -Wall -fpic -fvisibility=hidden -g
	$(LD) -fpic -fvisibility=hidden -shared *.o -o $(OUT_DIR)/scheduler.qkr

all: scheduler

clean:
	rm -f scheduler.o
	rm -f scheduler.qmo
