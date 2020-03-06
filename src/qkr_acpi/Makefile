CC = x86_64-elf-gcc
AS = x86_64-elf-as
LD = x86_64-elf-ld
OUT_DIR = ../_output/system
module1:
	$(CC) -c *.c -fvisibility=hidden -fpic -g -o acpi.o
	$(LD) -fpic -shared acpi.o -o $(OUT_DIR)/acpi.qkr

all: module1

clean:
	rm -f acpi.o
	rm -f $(OUT_DIR)/acpi.qkr
