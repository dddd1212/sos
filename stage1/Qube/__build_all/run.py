import os

def unmount():
    file("diskpartscript","wb").write(
r"""select vdisk file="%s\disk.vhd"
select partition=1
detach vdisk
"""%(os.path.abspath(DISK_FOLDER)))
    os.system("diskpart /s diskpartscript >> log.txt")

DISK_FOLDER = os.path.abspath("../_output/disk")
DISK_FILEPATH = os.path.abspath("../_output/disk/bochs_q")
BOOTLOADER_OUT = os.path.abspath("../bootloader/boot")

# copy the disk to the file for bochs:
#d = open(r"\\.\q:","rb")
unmount()
d = open(r"E:\gilad\sos-git2\sos\stage1\Qube\_output\disk\disk.vhd","r+b")
#f = open(DISK_FILEPATH, "wb")
#f.write(d.read())
#f.seek(0x1be)
#f.write("\xff") # make it bootable

d.seek(0x10000 + 90)
d.write(open(BOOTLOADER_OUT,"rb").read()[90:])
d.close()
#d.close()
os.system(r'"C:\Program Files (x86)\Bochs-2.6.8\bochsdbg.exe" -f vhd_disk.bxrc -rc debug_pre_funcs.txt')
raw_input()