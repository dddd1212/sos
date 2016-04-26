import os
import sys
import time

def create():
    file("diskpartscript","wb").write(
r"""create vdisk file="%s\disk.vhd" maximum=520
attach vdisk
create partition primary
format FS=FAT32 QUICK
detach vdisk
"""%(os.path.abspath(".")))
    os.system("diskpart /s diskpartscript >> log.txt")
    f = file("disk.vhd","r+")
    f.seek(0x1be)
    x = ord(f.read(1))|0x80
    f.seek(0x1be)
    f.write(chr(x))
    f.close()

def unmount():
    file("diskpartscript","wb").write(
r"""select vdisk file="%s\disk.vhd"
select partition=1
remove mount=%s\mount
detach vdisk
"""%(os.path.abspath("."),os.path.abspath(".")))
    os.system("diskpart /s diskpartscript >> log.txt")
    os.system("rmdir mount")
    
def mount():
    os.system("md mount")
    file("diskpartscript","wb").write(
r"""select vdisk file="%s\disk.vhd"
attach vdisk
select partition=1
assign mount=%s\mount
"""%(os.path.abspath("."),os.path.abspath(".")))
    os.system("diskpart /s diskpartscript >> log.txt")

def install_boot(boot_path):
    f = file("disk.vhd","r+b")
    f.seek(0x10000+90)
    f.write(file(boot_path,"rb").read()[90:])
    f.close()
    
def main(argv):
    if not ((len(argv)==2 and argv[1] in ["create","mount","unmount"]) or (len(argv)==3 and argv[1] in ["install_boot"])):
        print "usage: createhd.py create/mount/unmount/install_boot <bootfile>"
        exit(0)
    if argv[1] == "create":
        create()
    elif argv[1] == "mount":
        mount()
    elif argv[1] == "unmount":
        unmount()
    elif argv[1] == "install_boot":
        install_boot(argv[2])

if __name__ == "__main__":
    main(sys.argv)