CC = x86_64-elf-gcc.exe
AS = x86_64-elf-as.exe
LD = x86_64-elf-ld.exe
OUT_DIR = ../_output/system
libc:
	$(CC) -c *.c -fpic -g 
	$(LD) -fpic -fvisibility=hidden -shared *.o -o $(OUT_DIR)/libc.qkr

all: libc

clean:
	rm -f libc.o
	rm -f $(OUT_DIR)/libc.qkr
