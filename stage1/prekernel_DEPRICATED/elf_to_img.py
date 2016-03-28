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

EffSegment = nt("EffSegment", "type va data data_size offset")
EffRelocation = nt("EffRelocation", "va offset")
EffImport = nt("EffImport", "target_va target_offset name_va name_offset")
EffExport = nt("EffExport", "target_va target_offset name_va name_offset")

ADDR_SIZE = 8 # 64bit

EFF_HEADER_SIZE = 5 * 3 * ADDR_SIZE
SEGMENT_HEADER_SIZE = 4 * ADDR_SIZE
RELOCATION_SIZE = 2 * ADDR_SIZE
EXPORT_SIZE = 4 * ADDR_SIZE
IMPORT_SIZE = 4 * ADDR_SIZE

class Eff(object):
    # These 4 arguments determines the size of the header
    def __init__(self, num_of_segments, num_of_relocations, num_of_imports, num_of_exports):
        self.magic = "Eff_file"
        self.entry_point = DualAddress(-1,-1)
        
        self.num_of_segments = -1
        self.segments_addr = DualAddress(-1,-1)
        
        self.num_of_relocations = -1
        self.relocations_addr = DualAddress(-1,-1)
        
        self.num_of_exports = -1
        self.exports_addr = DualAddress(-1,-1)
        
        self.num_of_imports = -1
        self.imports_addr = DualAddress(-1,-1)
        
        self.segments = [] #Segments()
        self.relocations = [] #Relocations()
        self.imports = [] #Imports()
        self.exports = [] #Exports()
    
        #####
        self.imports_names_size = 0
        self.exports_names_size = 0
        #####
    def add_segment(self, type, va, data, data_size)
        data_size = ALIGN_UP(data_size, ADDR_SIZE) # pad the data with zeros to be aligned by ADDR_SIZE
        data = data.ljust(data_size)
        if seg.type & EffSegmentType_BYTES:
            self.data_size += len(data)
        assert va % 0x1000 == 0 # va aligned by page.
        
        self.segments.append(EffSegment(type=type,va=va,data=data,data_size=data_size, offset = -1))
    
    def add_relocation(self, dual_address_target):
        self.relocations.add_relocation(dual_address_target)
    
    def add_export(self, dual_address_target, name):
        self.exports_names_size += len(name) + 1 # to the \0
        self.exports.add_record(dual_address_target, name)
    
    def add_import(self, dual_address_target, name):
        self.imports_names_size += len(name) + 1 # to the \0
        self.imports.add_record(dual_address_target, name)

    def build_file(self):
        ## All the va addresses are if the file would loaded to va = 0.
        ## We assume that the EFF_HEADER will load to the va = 0, and that the file parts order in the file and in the memory is this:
        # | header | data and code segments | segments header | relocations | imports | exports |
        # We also build the file such the sugments header, relocations, imports and exports are loaded right one after another.

        self.base_header_offset = 0
        self.base_header_va = 0
        self.base_header_size = EFF_HEADER_SIZE
        
        self.data_offset = self.base_header_offset + self.base_header_size
        self.data_va = None # This is irellevant. We just need to know what is the end of va that the data will loaded to decide the va of the re
        self.data_size = self.data_size # Do nothing.
        
        self.segments_header_offset = self.data_offset + self.data_size
        self.segments_header_size = SEGMENT_HEADER_SIZE * len(self.segments)
        
        self.relocations_offset = self.segments_header_offset + self.segments_header_size
        self.relocations_size = RELOCATION_SIZE * len(self.relocations)
        
        self.imports_offset = self.relocations_offset + self.relocations_size
        self.imports_size = ALIGN_UP((IMPORT_SIZE * len(self.imports) + self.imports_names_size), ADDR_SIZE)
        
        self.exports_offset = self.imports_offset + self.imports_size
        self.exports_size = ALIGN_UP((EXPORT_SIZE * len(self.exports) + self.exports_names_size), ADDR_SIZE)
        
        # Now build the code/data segments with the segments headres:
        cur_offset = self.base_header_size + self.segments_header_size + self.relocations_size + self.imports_size + self.exports_size
        segments_headers = ""
        segments_data = ""
        for seg in self.segments:
            offset = -1
            if seg.type & EffSegmentType_BYTES:
                segments_data += seg.data
                seg.offset = cur_offset
                cur_offset += len(seg.data)
            segments_headers += struct.pack("<QQQQ",seg.type, seg.size, seg.offset, seg.va)
        
        # Add the imports, exports and relocations. This easy. we can use the sizes that we calculate, and the self.segments va and offset fields to 
        # calculate the va and offset to imports, exports and relocations.
        relocations = ""
        for rel in self.relocations:
            rel.offset = self.va_to_offset(rel.va)
            relocations += struct.pack("<QQ", rel.offset, rel.va)
        
        imports = ""
        names = ""
        names_va = 
        name_offset = 
        for imp in self.imports:
            imp.target_offset = self.va_to_offset(imp.target_va)
            imp.name_va = 
            imp.name_offset = self.va_to_offset(imp.name_va)
            imports += struct.pack("<QQQQ", imp.target_offset, imp.target_va, imp.name_offset, imp.name_va)
            
        
            
                
        
        
class Eff(object):
    def __init__(self):
        self.segments = []
        self.exports = {}
        self.imports = {}
        self.relocations = {}
        
    def add_segment(self, type, virtual_address, size, data):
        self.segments.append(EffSegment(type = type, virtual_address = virtual_address, size = size, data = data))
        if type & EffSegmentType_BYTES:
            assert size == len(data)
    
    def add_export(self, virtual_address, name):
        self.exports[name] = virtual_address
    
    def add_import(self, virtual_address, name):
        self.imports[name] = virtual_address
    
    def make_header(self):
        return "\xef"*0x1000
    
    def load(self, virtual_address):
        mem = {}
        mem[virtual_address] = self.make_header()
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
        
    def build_file(self):
        ret = ""
        ret = self.make_header()
        for seg in self.segments:
            


            
            
        
        
    


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
    