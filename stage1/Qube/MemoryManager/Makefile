CC = x86_64-elf-gcc.exe
AS = x86_64-elf-as.exe
LD = x86_64-elf-ld.exe
OUT_DIR = ../_output/system
memory_manager:
	$(CC) -c *.c -Wall -fpic -fvisibility=hidden -g
	$(LD) -fpic -fvisibility=hidden -shared *.o -o $(OUT_DIR)/memory_manager.qkr

all: memory_manager

clean:
	rm -f *.o
	rm -f $(OUT_DIR)/memory_manager.qkr
