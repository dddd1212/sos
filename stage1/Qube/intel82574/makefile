CC = x86_64-elf-gcc.exe
AS = x86_64-elf-as.exe
LD = x86_64-elf-ld.exe
OUT_DIR = ../_output/system
intel82574:
	$(CC) -c *.c -fpic -fvisibility=hidden -g -o intel82574.o
	$(LD) -fpic -fvisibility=hidden -shared intel82574.o -o $(OUT_DIR)/intel82574.qkr

all: intel82574

clean:
	rm -f intel82574.o
	rm -f intel82574.qmo
