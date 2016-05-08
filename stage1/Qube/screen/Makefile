CC = x86_64-elf-gcc.exe
AS = x86_64-elf-as.exe
LD = x86_64-elf-ld.exe
OUT_DIR = ../_output/system
module1:
	python ../common/exports.py
	$(CC) -c *.c -fpic -fvisibility=hidden -include exports.h -g -o screen.o
	$(LD) -fpic -fvisibility=hidden -shared screen.o -o $(OUT_DIR)/screen.qkr

all: module1

clean:
	rm -f screen.o
	rm -f $(OUT_DIR)/screen.qkr
