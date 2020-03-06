import os
import struct

######### BOOT LOADER CONFIGURATION #################
THIS_DIR = os.path.dirname(__file__)
BASE_DIR = os.path.join(THIS_DIR,"..")
OUT_DIR = os.path.join(BASE_DIR,"_output","x64","BootLoader")
REAL_MODE_ASM_PATH = os.path.join(BASE_DIR, "_tools","nasm","nasm.exe")
REAL_MODE_FILE_PATH = os.path.join(THIS_DIR, "real_mode.asm")
REAL_MODE_OUTPUT = os.path.join(OUT_DIR, "real_mode.bin")
LONG_MODE_OUTPUT = os.path.join(OUT_DIR, "bootloader.exe")
BOOT_LOADER_OUTPUT = os.path.join(OUT_DIR, "bootloader.bin")
#####################################################

def build():
    print "Assemble realmode.asm..."
    os.system("del %s"%BOOT_LOADER_OUTPUT)
    ret = os.system(REAL_MODE_ASM_PATH + " " + REAL_MODE_FILE_PATH + " -o " + REAL_MODE_OUTPUT)

    if ret != 0:
        print "Error while compiling REAL_MODE!"
        return 1
    
    # Read the exe file:
    long_mode = open(LONG_MODE_OUTPUT,"rb").read()
    
    # Got the PE header offset:
    pe_off = struct.unpack("<I", long_mode[0x3c:0x3c+4])[0]
    # verify that its PE
    #print len(long_mode),long_mode[0x0:0xb0+4].encode("hex")
    assert long_mode[pe_off:pe_off+4] == "PE\x00\x00"
    
    # read the num_of_sections and the size of the opt_header.
    num_of_sections = struct.unpack("<H", long_mode[pe_off+6:pe_off+8])[0]
    size_of_optional_header = struct.unpack("<H", long_mode[pe_off+0x14:pe_off+0x16])[0]

    # get from opt_header the entry_point_address
    opt_header = long_mode[pe_off+0x18:pe_off + 0x18 + size_of_optional_header]
    assert opt_header[0:2] == "\x0b\x02" # PE64 magic
    entry_point_address = struct.unpack("<I", opt_header[0x10:0x14])[0]

    # Find the ".text" section and get the code:
    section_header = long_mode[pe_off+0x18 + size_of_optional_header:]
    jump = None
    for i in xrange(num_of_sections):
        start = i*0x28 # size of section header.
        #print "header: ",section_header[start:start + 8]
        if section_header[start:start + 8] == ".text\x00\x00\x00":
            va, size, ptr, rel, line, num_rel, num_line = struct.unpack("<IIIIIHH",section_header[start + 0xc:start + 0x24])
            #print va, size, ptr, rel, line, num_rel, num_line
            assert num_rel == 0 # no relocations
            assert rel == 0 # not relocations.
            code = long_mode[ptr:ptr+size] # The raw code.
            
            # Validate that the entry point is in the .text section
            assert va < entry_point_address < va + size
            jump = entry_point_address - va # num of opcode to jump to the entry point.
            break
    if jump == None:
        raise
    # Assemble the jump opcode:
    temp = open("temp.temp","wb")
    temp.write("bits 64\r\njmp $+%d\r\n"%(jump+2))
    temp.close()
    os.system(REAL_MODE_ASM_PATH + " temp.temp -o temp.temp.out")
    f = open("temp.temp.out","rb")
    jump_opcode = f.read()
    f.close()
    os.system("del temp.temp")
    os.system("del temp.temp.out")

    # build the whole image:
    out = open(BOOT_LOADER_OUTPUT,"wb")
    real_mode = open(REAL_MODE_OUTPUT,"rb").read()
    #real_mode = ""
    out.write(real_mode + jump_opcode + code)
    out.close()





        
    
    
if __name__ == "__main__":
    build()