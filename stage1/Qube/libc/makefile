CC = x86_64-elf-gcc.exe
AS = x86_64-elf-as.exe
LD = x86_64-elf-ld.exe
libc:
	python ../common/exports.py
	$(CC) -c *.c -fpic -fvisibility=hidden -o libc.o
	$(LD) -fpic -fvisibility=hidden -shared libc.o -o libc.qmo

all: libc

clean:
	rm -f libc.o
	rm -f libc.qmo
