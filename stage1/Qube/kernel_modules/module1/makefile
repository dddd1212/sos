CC = x86_64-elf-gcc.exe
AS = x86_64-elf-as.exe
LD = x86_64-elf-ld.exe
module1:
	python ../common/exports.py
	$(CC) -c module1_funcs.c -fpic -fvisibility=hidden
	$(LD) -fpic -fvisibility=hidden -shared module1_funcs.o -o libmodule1_funcs.so

all: module1

clean:
	rm -f module1_funcs.o
	rm -f libmodule1_funcs.so
