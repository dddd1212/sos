CC = x86_64-elf-gcc.exe
AS = x86_64-elf-as.exe
OUTPUT = ../_output
INTERMEDIATE = ../_output/bootloader

boot: *.c *.h
	mkdir $(INTERMEDIATE)
	$(AS) real_mode.asm -o $(INTERMEDIATE)/realmode
	$(CC) *.c -T linker.ld -o $(INTERMEDIATE)/longmode -ffreestanding -nostdlib -mno-red-zone -fno-exceptions -fno-asynchronous-unwind-tables -lgcc
	cat $(INTERMEDIATE)/realmode $(INTERMEDIATE)/longmode > $(OUTPUT)/bootloader.bin
all: boot

clean:
	rm -f -r $(INTERMEDIATE)
	rm -f $(OUTPUT)/bootloader.bin