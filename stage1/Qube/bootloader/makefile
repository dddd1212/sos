SRC_FILES := $(filter-out long_mode_main.c,$(wildcard *.c))
boot: *.c *.h
	x86_64-elf-gcc -c long_mode_main.c -ffreestanding -nostdlib -mno-red-zone -fno-exceptions -fno-asynchronous-unwind-tables -lgcc
	x86_64-elf-gcc $(SRC_FILES) -o boot -T linker.ld -ffreestanding -nostdlib -mno-red-zone -fno-exceptions -fno-asynchronous-unwind-tables -lgcc