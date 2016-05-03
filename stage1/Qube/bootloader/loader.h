#ifndef __LOADER_H__
#define __LOADER_H__

typedef int(*EntryPoint)(KernelGlobalData * kgd);

struct STAGE0BootModule {
	char * file_name; // pointer to the module file name
	unsigned int file_pages; // num of pages the file need.
	struct Elf64Header * file_data; // data of the eff file.
	void * module_base; // pointer to the loaded module.
	EntryPoint entry_point; // module entry point
};

#define Elf64_Off uint64
#define Elf64_Addr uint64
#define Elf64_Word uint32
#define Elf64_Half uint16
#define Elf64_Xword uint64
typedef enum {
	PT_NULL = 0,
	PT_LOAD = 1,
	PT_DYNAMIC = 2,
	PT_INERP = 3,
	PT_NOTE = 4,
	PT_SHLIB = 5,
	PT_PHDR = 6,
	PT_LOOS = 0x60000000,
	PT_HIOS = 0x6fffffff,
	PT_LOPROC = 0x70000000,
	PT_HIPROC = 0x7fffffff
} p_type64_e;
typedef enum {
	PF_None = 0,
	PF_Exec = 1,
	PF_Write = 2,
	PF_Write_Exec = 3,
	PF_Read = 4,
	PF_Read_Exec = 5,
	PF_Read_Write = 6,
	PF_Read_Write_Exec = 7
} p_flags64_e;
typedef enum { // 64bit enum
	SF64_None = 0LL,
	SF64_Exec = 1,
	SF64_Alloc = 2,
	SF64_Alloc_Exec = 3,
	SF64_Write = 4,
	SF64_Write_Exec = 5,
	SF64_Write_Alloc = 6,
	SF64_Write_Alloc_Exec = 7
} s_flags64_e;
typedef enum {
	SHT_NULL = 0, /* Inactive section header */
	SHT_PROGBITS = 1, /* Information defined by the program */
	SHT_SYMTAB = 2, /* Symbol table - not DLL */
	SHT_STRTAB = 3, /* String table */
	SHT_RELA = 4, /* Explicit addend relocations, Elf64_Rela */
	SHT_HASH = 5, /* Symbol hash table */
	SHT_DYNAMIC = 6, /* Information for dynamic linking */
	SHT_NOTE = 7, /* A Note section */
	SHT_NOBITS = 8, /* Like SHT_PROGBITS with no data */
	SHT_REL = 9, /* Implicit addend relocations, Elf64_Rel */
	SHT_SHLIB = 10, /* Currently unspecified semantics */
	SHT_DYNSYM = 11, /* Symbol table for a DLL */

	SHT_LOOS = 0x60000000, /* Lowest OS-specific section type */
	SHT_HIOS = 0x6fffffff, /* Highest OS-specific section type */

	SHT_LOPROC = 0x70000000, /* Lowest processor-specific section type */
	SHT_HIPROC = 0x7fffffff  /* Highest processor-specific section type */
} s_type64_e;
struct Elf64ProgramHeader {
	p_type64_e p_type;
	p_flags64_e p_flags;
	Elf64_Off offset; // offset from file begin.
	Elf64_Addr p_vaddr;
	Elf64_Addr p_paddr; // unused.
	Elf64_Xword p_filesz; // Segment file length
	Elf64_Xword p_memsz; // Segment ram length
	Elf64_Xword p_align;
};

struct Elf64SectionHeader {
	uint32 s_name; // name offset from the string table
	s_type64_e s_type;
	s_flags64_e s_flags;
	Elf64_Addr s_addr;
	Elf64_Off s_offset;
	Elf64_Xword s_size;
	Elf64_Word s_link;
	Elf64_Word s_info;
	Elf64_Xword s_addralign;
	Elf64_Xword s_entsize;

};

struct Elf64RelaStruct {
	Elf64_Addr r_addr;
	Elf64_Word r_type;
	Elf64_Word r_index;
	Elf64_Addr pad;
};
struct Elf64Header {
	char e_ident[16];
	uint16 e_type;
	uint16 e_machine;
	uint32 e_version;
	Elf64_Addr e_entry; // entry point
	Elf64_Off e_phoff; // Program header offset in file.
	Elf64_Off e_shoff; // Section header offset in file.
	Elf64_Word e_flags;
	Elf64_Half e_ehsize; // Elf header size
	Elf64_Half e_phentsize; // Program header entry size in file
	Elf64_Half e_phnum; // Number of program header entries
	Elf64_Half e_shentsize; // Section header entry size
	Elf64_Half e_shnum; // Nmber of section header entries
	Elf64_Half e_shstrndx; // String table index
};

struct Elf64Symbol {
	uint32 sym_name; // offset from string table.
	char sym_info;
	unsigned char sym_other;
	Elf64_Half sym_shndx;
	Elf64_Addr sym_value;
	Elf64_Xword sym_size;
};



int load_modules_and_run_kernel(KernelGlobalData * kgd, struct STAGE0BootModule * boot_modules, BootLoaderAllocator * boot_loader_allocator, int num_of_modules);
int count_sections_by_type(struct STAGE0BootModule * boot_modules, s_type64_e type);
void * find_section_by_name(struct STAGE0BootModule * boot_modules, char * name, Elf64_Xword * size_out);
void * find_section_by_type(struct STAGE0BootModule * boot_modules, s_type64_e type, Elf64_Xword * size_out);
Elf64_Addr find_symbol(KernelGlobalData * kgd, char * sym_name);
int add_to_symbol_table(KernelGlobalData * kgd, char * sym_name, Elf64_Addr sym_addr);


#endif // __LOADER_H__


