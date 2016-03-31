CC = x86_64-elf-gcc.exe
AS = x86_64-elf-as.exe

boot: *.c *.h real_mode.asm
	$(AS) real_mode.asm -o real_mode.o
	$(CC) *.c -o boot -T linker.ld -ffreestanding -nostdlib -mno-red-zone -fno-exceptions -fno-asynchronous-unwind-tables -lgcc

all: boot

clean:
	rm -f *.o
	rm -f boot