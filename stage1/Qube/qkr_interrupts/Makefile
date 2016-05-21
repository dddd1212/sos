CC = x86_64-elf-gcc.exe
AS = x86_64-elf-as.exe
LD = x86_64-elf-ld.exe
OUT_DIR = ../_output/system
INTER_DIR = ../_output/system/inter
interrupts:
	python generate_isr.py
	$(AS) isrs.S -g -o $(INTER_DIR)/isrs.o
	$(CC) -c *.c -fpic -fvisibility=hidden -g -o $(INTER_DIR)/interrupts.o
	$(LD) -fpic -fvisibility=hidden -shared $(INTER_DIR)/isrs.o $(INTER_DIR)/interrupts.o -o $(OUT_DIR)/interrupts.qkr

all: interrupts

clean:
	rm -f $(INTER_DIR)/interrupts.o
	rm -f $(INTER_DIR)/isrs.o
	rm -f $(INTER_DIR)/interrupts.qmo
