CC = x86_64-elf-gcc.exe
AS = x86_64-elf-as.exe
LD = x86_64-elf-ld.exe
OUT_DIR = ../_output/system
libc:
	python ../common/exports.py
	$(CC) -c *.c -fpic -fvisibility=hidden -o libc.o
	$(LD) -fpic -fvisibility=hidden -shared libc.o -o $(OUT_DIR)/libc.qkr

all: libc

clean:
	rm -f libc.o
	rm -f $(OUT_DIR)/libc.qkr
