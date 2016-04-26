import os

def unmount():
    file("diskpartscript","wb").write(
r"""select vdisk file="%s\disk.vhd"
select partition=1
detach vdisk
"""%(os.path.abspath(DISK_FOLDER)))
    os.system("diskpart /s diskpartscript >> log.txt")
try:
	DISK_FOLDER = os.path.abspath("../_output/_disk")
	DEBUGGER_PATH = '"e:\\gilad\\Affinic Debugger\\adg.exe"'
	DISK_FILEPATH = os.path.abspath("../_output/_disk/disk.vhd")
	BOOTLOADER_OUT = os.path.abspath("../_output/bootloader/boot.bin")

	unmount()
	d = open(DISK_FILEPATH,"r+b")
	d.seek(0x10000 + 90)
	d.write(open(BOOTLOADER_OUT,"rb").read()[90:])
	d.close()

	os.system('start "" %s -x gdbinit'%DEBUGGER_PATH)
	os.system('start "" "C:\\Program Files (x86)\\Bochs-2.6.8\\bochs_gdb.exe" -q -f vhd_disk_gdb.bxrc')
	
except:
	print "Exception..."
	import traceback
	traceback.print_exc()
	raw_input()
#raw_input()