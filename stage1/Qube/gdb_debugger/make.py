# The file do the whole making process including make a disk image and put the nessecery file in it.
import os
import sys

DISK_FOLDER = os.path.abspath("../_output/_disk")
DISK_FOLDER_MOUNT = os.path.abspath("mount")
DISK_FILEPATH = os.path.abspath("../_output/_disk/disk.vhd")
BOOTLOADER_OUT = os.path.abspath("../_output/bootloader/boot.bin")

def mount():
    #import code
    #code.interact(local = locals())
    print "mount called"
    os.system("md mount")
    open("diskpartscript","wb").write(
r"""select vdisk file="%s\disk.vhd"
attach vdisk
select partition=1
assign mount=%s
"""%(os.path.abspath(DISK_FOLDER ),os.path.abspath("mount")))
    os.system("diskpart /s %s >> %s"%(os.path.abspath("diskpartscript"),os.path.abspath("diskpartscriptlog.txt")))

def unmount():
    open("diskpartscript","wb").write(
r"""select vdisk file="%s\disk.vhd"
select partition=1
detach vdisk
"""%(os.path.abspath(DISK_FOLDER)))
    os.system("diskpart /s %s >> %s"%(os.path.abspath("diskpartscript"),os.path.abspath("diskpartscriptlog.txt")))

def install_boot():
    f = open(DISK_FILEPATH,"r+b")
    f.seek(0x10000+90)
    f.write(open(BOOTLOADER_OUT,"rb").read()[90:])
    f.close()
    print "here"

def write_to_disk(offset, data, letter = 'q'):
    disk = open(r"\\.\%s:"%letter,"r+b")
    offset_from = offset / 512 * 512
    offset_to = (offset + len(data)) / 512 * 512 + 512
    disk.seek(offset_from)
    orig_data = disk.read(offset_to - offset_from)
    assert len(orig_data) == offset_to - offset_from
    write_data = orig_data[:offset - offset_from] + data + orig_data[offset - offset_from + len(data):]
    assert len(orig_data) == len(write_data)
    print offset_from
    disk.seek(offset_from)
    disk.write(write_data)
    #for i in xrange(len(write_data)/512):
    #    disk.write(write_data[i*512:(i+1)*512])
    #    print "write %d"%i
    disk.close()

if sys.argv[1] in ('build','rebuild'):
    #import pdb
    #pdb.set_trace()
    CUR_DIR = os.path.abspath('.')
    LAST_COMPILE_TIMES_FILE = os.path.abspath('last_compile_times.txt')

    STATIC_FILES_PATH = os.path.abspath("../_static_files/")
    SYSTEM_PATH = os.path.abspath("../_output/system/")

    ### MODULES: 
    BOOTLOADER_DIR = os.path.abspath("../bootloader")
    BOOTLOADER_OUT = os.path.abspath("../_output/bootloader/boot.bin")
    print BOOTLOADER_OUT

    #####

    mount()
    ## Read last_compile_times file:
    if not os.path.exists(LAST_COMPILE_TIMES_FILE):
        open(LAST_COMPILE_TIMES_FILE,"wb").close()
    data = open(LAST_COMPILE_TIMES_FILE,"rb").read()
    last_compile_times = {}
    for line in data.split("\n"):
        if line == '':
            continue
        tokens = line.strip().split("=")
        last_compile_times[tokens[0]] = tokens[1]
    #################

    ### Compile bootloader ####
    os.chdir(BOOTLOADER_DIR)
    if False: # We use dependecies instead
        ret = os.system("make all")
    this_time = str(os.path.getmtime(BOOTLOADER_OUT))
    if last_compile_times.has_key("bootloader"):
        last_time = last_compile_times["bootloader"]
    else:
        last_time = 0
    os.chdir(CUR_DIR)


    # If the compile output changed:
    if False and this_time != last_time: # The output modified.
        os.system("format q: /FS:FAT32 /Q /A:512 /y")
        print "boot loader modified!"
        last_compile_times["bootloader"] = str(this_time)
        write_to_disk(90, open(BOOTLOADER_OUT,"rb").read()[90:])

    # delete all the files in the disk:
    os.chdir(DISK_FOLDER_MOUNT)
    os.system("del *.* /Q")
    os.chdir(CUR_DIR)

    files = [f for f in os.listdir(STATIC_FILES_PATH) if os.path.isfile(os.path.join(STATIC_FILES_PATH,f)) and f not in ("_static_files.vcxproj", "_static_files.vcxproj.filters")]
    system_files = [f for f in os.listdir(SYSTEM_PATH) if os.path.isfile(os.path.join(SYSTEM_PATH,f)) and f not in ("_static_files.vcxproj", "_static_files.vcxproj.filters")]
    
    print files
    # copy the static files to the disk:
    for file in files:
        open(os.path.join(DISK_FOLDER_MOUNT, file),"wb").write(open(os.path.join(STATIC_FILES_PATH, file),"rb").read())
        print "write file %s!"%file
    print system_files
    # copy the system files to the disk:
    open(os.path.join(SYSTEM_PATH, "tmplongname.fil"),"wb").write("Magic482554427352")
    if os.path.isfile(os.path.join(SYSTEM_PATH, "tmplon~1.fil")):
        assert (open(os.path.join(SYSTEM_PATH, "tmplon~1.fil"),"rb").read() == "Magic482554427352")
        create8dot3name = False
    else:
        create8dot3name = True
    print "create8dot3name:",create8dot3name
    os.remove(os.path.join(SYSTEM_PATH, "tmplongname.fil"))
    for file in system_files:
        print "debug: ", file
        if create8dot3name and len(file[:file.find('.')])>8: open(os.path.join(SYSTEM_PATH, file[:6]+"~1"+file[file.find("."):]),"wb").write(open(os.path.join(SYSTEM_PATH, file),"rb").read())
        if '~' in file: continue
        open(os.path.join(DISK_FOLDER_MOUNT, file),"wb").write(open(os.path.join(SYSTEM_PATH, file),"rb").read())
        print "write file %s!"%file
    try:
        ret = ""
        for i in last_compile_times.items():
            ret += "=".join(i) + "\n"
        open(LAST_COMPILE_TIMES_FILE,"wb").write(ret)
    except:
        print "Error in writing to last_compile_times file!"
    print "Writing new compile times to file!"
    unmount()
    install_boot()
