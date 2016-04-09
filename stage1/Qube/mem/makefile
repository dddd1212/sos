CC = x86_64-elf-gcc.exe
AS = x86_64-elf-as.exe
LD = x86_64-elf-ld.exe
mem:
	python ../common/exports.py
	$(CC) -c *.c -fpic
	$(LD) -fpic -shared *.o -o mem.so

all: mem

clean:
	rm -f *.o
	rm -f mem.so
