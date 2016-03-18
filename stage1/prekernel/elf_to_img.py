import struct
from collections import namedtuple as nt
PROG_HDR_TYPES = \
{0:"PT_NULL",
 1:"PT_LOAD",	
 2:"PT_DYNAMIC",
 3:"PT_INTERP"	,
 4:"PT_NOTE"	,
 5:"PT_SHLIB"	,
 6:"PT_PHDR"	,
 7:"PT_TLS"	,
 0x60000000:"PT_LOOS"	,
 0x6fffffff:"PT_HIOS"	,
 0x70000000:"PT_LOPROC"	,
 0x7fffffff:"PT_HIPROC",
 0x6474e550:"PT_GNU_EH_FRAME",
 0x6474e551:"PT_GNU_STACK"}
 
SECTION_HDR_TYPES = \
{0:"SHT_NULL"	    ,    
1:"SHT_PROGBITS"  	,
2:"SHT_SYMTAB"    	,
3:"SHT_STRTAB"    	,
4:"SHT_RELA"	     ,   
5:"SHT_HASH"	      ,  
6:"SHT_DYNAMIC"	    ,
7:"SHT_NOTE"	     ,   
8:"SHT_NOBITS"	    ,
9:"SHT_REL"	        ,
10:"SHT_SHLIB"	     ,   
11:"SHT_DYNSYM"	    ,
14:"SHT_INIT_ARRAY"	,
15:"SHT_FINI_ARRAY"	,
16:"SHT_PREINIT_ARRAY"	,
17:"SHT_GROUP"	        ,
18:"SHT_SYMTAB_SHNDX"	,
0x60000000:"SHT_LOOS"	 ,       
0x6fffffff:"SHT_HIOS"	  ,      
0x70000000:"SHT_LOPROC"	   , 
0x7fffffff:"SHT_HIPROC"	    ,
0x80000000:"SHT_LOUSER"	    ,
0xffffffff:"SHT_HIUSER"	    }

EffSegmentType_BYTES = 16
EffSegmentType_BSS = 8

EffSegmentType_R = 4
EffSegmentType_W = 2
EffSegmentType_X = 1

EffSegment = nt("EffSegment", "type virtual_address size data")

class Eff(object):
    def __init__(self):
        self.segments = []
    
    def add_segment(self, type, virtual_address, size, data):
        self.segments.append(EffSegment(type = type, virtual_address = virtual_address, size = size, data = data))
        if type & EffSegmentType_BYTES:
            assert size == len(data)
        
    def load(self, virtual_address):
        mem = {}
        for seg in self.segments:
            if seg.type & EffSegmentType_BYTES:
                mem[seg.virtual_address] = seg.data
            elif seg.type & EffSegmentType_BSS:
                mem[seg.virtual_address] = "\x00"*seg.size
        k = mem.keys()
        k.sort()
        last_addr = k[0]
        ret = ""
        for i in k:
            ret += "\x00"*(i-last_addr) + mem[i]
            
        # TODO: handle relocations here!
        # TODO: handle imports / exports??!?!?
        return ret
        


            
            
        
        
    


def elf2EffObj(elf_data):
    eff = Eff()
    
    e_ident = elf_data[:0x10]
    EI_MAG, EI_CLASS, EI_DATA, EI_VERSION, EI_OSABI, EI_ABIVERSION, EI_PAD = struct.unpack("4sBBBBB7s", e_ident)
    assert EI_MAG == "\x7fELF"
    assert EI_CLASS == 2 # 64bit
    assert EI_DATA == 1 # little endian
    assert EI_VERSION == 1 # elf version
    e_type, e_machine, e_version = struct.unpack("<HHI", elf_data[0x10:0x18])
    assert e_machine == 0x3e # x86-64bit
    assert e_version == 1 # another elf version
    e_entry, ephoff, eshoff = struct.unpack("<QQQ", elf_data[0x18:0x30])
    e_flags, e_ehsize, e_phentsize, e_phnum, e_shentsize, e_shnum, e_shstrndx = struct.unpack("<IHHHHHH", elf_data[0x30:0x40])
    
    
    program_headers = elf_data[ephoff:ephoff + e_phentsize*e_phnum]
    for i in xrange(e_phnum):
        program_header = program_headers[i*e_phentsize:(i+1)*e_phentsize]
        p_type, p_flags, p_offset, p_vaddr, p_paddr, p_filesz, p_memsz, p_align = struct.unpack("<IIQQQQQQ",program_header)
        assert p_flags < 8 # just XWR flags
        if PROG_HDR_TYPES[p_type] == "PT_LOAD":
            if p_filesz > 0: # Handle .text segments
                eff.add_segment(type = EffSegmentType_BYTES | p_flags, virtual_address = p_vaddr, size = p_filesz, data = elf_data[p_offset:p_offset+p_filesz])
            if p_memsz > p_filesz: # Hnadle .bss segments
                eff.add_segment(type = EffSegmentType_BSS | p_flags, virtual_address = p_vaddr + p_filesz, size = p_memsz - p_filesz, data = "")

        elif PROG_HDR_TYPES[p_type] in ("PT_NOTE", "PT_GNU_STACK"): # Ignore these segments.
            continue
        else:
            print "Unsupported program header: %s"%PROG_HDR_TYPES[p_type]
            raise
    
    section_headers = elf_data[eshoff:eshoff + e_shentsize*e_shnum]
    #print "num of section headers: %d"%e_shnum
    
    for i in xrange(e_shnum):
        section_header = section_headers[i*e_shentsize:(i+1)*e_shentsize]
        sh_name, sh_type, sh_flags, sh_addr, sh_offset, sh_size, sh_link, sh_info, sh_addralign, sh_entsize = struct.unpack("<IIQQQQIIQQ", section_header)
        if SECTION_HDR_TYPES[sh_type] in ("SHT_NULL","SHT_NOTE","SHT_PROGBITS","SHT_NOBITS","SHT_STRTAB","SHT_SYMTAB",):
            continue
        else:
            print "Unsupported section header: %s"%SECTION_HDR_TYPES[sh_type]
            raise
        #print struct.unpack("<IIQQQQIIQQ", section_header)
        #print "section header type: %s"%SECTION_HDR_TYPES[sh_type]
    
    return eff
    
    
    
    
    
    
    
eff = elf2EffObj(open(r"c:\users\gilad\desktop\out.out","rb").read())
open("out.img","wb").write(eff.load(0x80000000))
    