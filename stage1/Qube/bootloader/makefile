TOOLCHAIN = C:/cygwin64/usr/local/cross64/bin
CC = $(TOOLCHAIN)/x86_64-elf-gcc.exe

boot: *.c *.h
	$(CC) *.c -T linker.ld -o boot -ffreestanding -nostdlib -mno-red-zone -fno-exceptions -fno-asynchronous-unwind-tables -lgcc
all: boot