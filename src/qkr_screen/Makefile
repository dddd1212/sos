CC = x86_64-elf-gcc
AS = x86_64-elf-as
LD = x86_64-elf-ld
OUT_DIR = ../_output/system
module1:
	$(CC) -c *.c -fvisibility=hidden -fpic -g -o screen.o
	$(LD) -fpic -shared screen.o -o $(OUT_DIR)/screen.qkr

all: module1

clean:
	rm -f screen.o
	rm -f $(OUT_DIR)/screen.qkr
