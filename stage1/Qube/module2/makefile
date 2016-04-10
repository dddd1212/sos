CC = x86_64-elf-gcc.exe
AS = x86_64-elf-as.exe
LD = x86_64-elf-ld.exe
module1:
	python ../common/exports.py
	$(CC) -c module2_funcs.c -fpic -fvisibility=hidden
	$(LD) -fpic -fvisibility=hidden -shared module2_funcs.o -o libmodule2_funcs.so -L../module1/ -lmodule1_funcs

all: module1

clean:
	rm -f module2_funcs.o
	rm -f libmodule2_funcs.so
