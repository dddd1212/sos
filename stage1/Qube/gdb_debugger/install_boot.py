import os
DISK_FILEPATH = os.path.abspath("../_output/_disk/disk.vhd")
BOOTLOADER_OUT = os.path.abspath("../_output/bootloader/boot.bin")
f = open(DISK_FILEPATH,"r+b")
f.seek(90)
f.write(open(BOOTLOADER_OUT,"rb").read()[90:0x200])
f.seek(0x400)
f.write(open(BOOTLOADER_OUT,"rb").read()[0x400:])
f.close()