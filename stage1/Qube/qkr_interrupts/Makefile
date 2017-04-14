CC = x86_64-elf-gcc
AS = x86_64-elf-as
LD = x86_64-elf-ld
OUT_DIR = ../_output/system
INTER_DIR = ../_output/system/inter
interrupts:
	python generate_isr.py
	$(AS) isrs.S -g -o $(INTER_DIR)/isrs.o
	$(CC) -c *.c -Wall -fpic -fvisibility=hidden -g
	$(LD) -fpic -fvisibility=hidden -shared $(INTER_DIR)/isrs.o *.o -o $(OUT_DIR)/intrpts.qkr

all: interrupts

clean:
	rm -f $(INTER_DIR)/*.o
	rm -f $(OUT_DIR)/intrpts.qkr
