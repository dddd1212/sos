boot: *.c *.h
	x86_64-elf-gcc *.c -T linker.ld -o boot -ffreestanding -nostdlib -mno-red-zone -fno-exceptions -fno-asynchronous-unwind-tables -lgcc