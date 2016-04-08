CC = x86_64-elf-gcc.exe
AS = x86_64-elf-as.exe
OBJCOPY = x86_64-elf-objcopy.exe
NM = x86_64-elf-nm.exe
boot: *.c *.h real_mode.asm
	$(AS) real_mode.asm -o real_mode.o
	$(CC) *.c -T linker.ld -g -o boot.elf -ffreestanding -nostdlib -mno-red-zone -fno-exceptions -fno-asynchronous-unwind-tables -lgcc
	$(OBJCOPY) -O binary boot.elf boot
	sh create_bochs_sym_file.sh boot.elf
all: boot

clean:
	rm -f *.o
	rm -f boot