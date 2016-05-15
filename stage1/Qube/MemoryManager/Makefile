CC = x86_64-elf-gcc.exe
AS = x86_64-elf-as.exe
LD = x86_64-elf-ld.exe
OUT_DIR = ../_output/system
memory_manager:
	$(CC) -c *.c -fpic -fvisibility=hidden -g -o memory_manager.o
	$(LD) -fpic -fvisibility=hidden -shared memory_manager.o -o $(OUT_DIR)/memory_manager.qkr

all: memory_manager

clean:
	rm -f memory_manager.o
	rm -f memory_manager.qmo
