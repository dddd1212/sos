SRC_FILES := $(filter-out long_mode_main.c,$(wildcard *.c))
boot: *.c *.h real_mode.asm
	x86_64-elf-as real_mode.asm -o real_mode.o
	x86_64-elf-gcc *.c -o boot -T linker.ld -ffreestanding -nostdlib -mno-red-zone -fno-exceptions -fno-asynchronous-unwind-tables -lgcc