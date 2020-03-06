CC = $(TOOLCHAIN_PREFIX)-gcc
AS = $(TOOLCHAIN_PREFIX)-as
OBJCOPY = $(TOOLCHAIN_PREFIX)-objcopy
OUT_DIR = ../_output/bootloader
INTER_DIR = ../_output/bootloader/inter

boot: *.c *.h real_mode.asm
	mkdir -p $(OUT_DIR)
	mkdir -p $(INTER_DIR)
	$(AS) real_mode.asm -g -o $(INTER_DIR)/real_mode.o
	$(CC) *.c -Wall -T linker.ld -g -o $(OUT_DIR)/boot.elf -ffreestanding -nostdlib -mno-red-zone -fno-exceptions -fno-asynchronous-unwind-tables -lgcc
	$(OBJCOPY) -O binary $(OUT_DIR)/boot.elf $(OUT_DIR)/boot.bin
all: boot

clean:
	rm -f $(OUT_DIR)/boot.elf
	rm -f $(OUT_DIR)/boot.bin
	rm -f $(INTER_DIR)/real_mode.o
	
