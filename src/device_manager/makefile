CC = x86_64-elf-gcc
AS = x86_64-elf-as
LD = x86_64-elf-ld
OUT_DIR = ../_output/system
device_manager:
	$(CC) -c *.c -fpic -fvisibility=hidden -g -o device_manager.o
	$(LD) -fpic -fvisibility=hidden -shared device_manager.o -o $(OUT_DIR)/device_manager.qkr

all: device_manager

clean:
	rm -f device_manager.o
	rm -f device_manager.qmo
