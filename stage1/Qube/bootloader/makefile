CC = x86_64-elf-gcc.exe
AS = x86_64-elf-as.exe
OBJCOPY = x86_64-elf-objcopy.exe
NM = x86_64-elf-nm.exe
OUT_DIR = ../_output/bootloader
INTER_DIR = ../_output/bootloader/inter

boot: *.c *.h real_mode.asm
	$(AS) real_mode.asm -g -o $(INTER_DIR)/real_mode.o
	$(CC) *.c -Wall -T linker.ld -g -o $(OUT_DIR)/boot.elf -ffreestanding -nostdlib -mno-red-zone -fno-exceptions -fno-asynchronous-unwind-tables -lgcc
	$(OBJCOPY) -O binary $(OUT_DIR)/boot.elf $(OUT_DIR)/boot.bin
all: boot

clean:
	rm -f $(OUT_DIR)/boot.elf
	rm -f $(OUT_DIR)/boot.bin
	rm -f $(INTER_DIR)/real_mode.o
	
