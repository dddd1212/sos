CC = x86_64-elf-gcc.exe
AS = x86_64-elf-as.exe
LD = x86_64-elf-ld.exe
OUT_DIR = ../_output/system
kernel:
	$(CC) -c *.c -Wall -fpic -fvisibility=hidden -g -o kernel.o
	$(LD) -fpic -fvisibility=hidden -shared kernel.o -o $(OUT_DIR)/kernel.qkr

all: kernel

clean:
	rm -f kernel.o
	rm -f kernel.qmo
