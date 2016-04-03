
#include "Qube.h"
#include "hd.h"
#include "mem.h"
#include "loader.h"
#include "libc.h"
// kgd - pointer to KernelGlobalData\
// boot_modules - ptr to array of STAGE0BootModules
// num_of_modules - boot_modules entries
int load_modules(struct KernelGlobalData * kgd, struct STAGE0BootModule * boot_modules, BootLoaderAllocator * boot_loader_allocator, int num_of_modules) {
	struct STAGE0BootModule * boot_module;
	struct Elf64ProgramHeader * ph;
	int i, j;
	char * module_base, ret;
	char string_strtab[] = { '.','s','t','r','t','a','b','\x00' };
	char export_prefix[] = { 'E','X','P','_' };
	Elf64_Xword size;
	// First - load the segments, handle exports, and reloacations
	
	for (i = 0, boot_module = boot_modules; i < num_of_modules; i++, boot_module++) {
		// Calc the size of virtual memory we need to reserve:
		// Iterate over the program header tables:
		Elf64_Xword reserved_start = 0xffffffffffffffff;
		Elf64_Xword reserved_end = 0;
		for (j = 0, ph = (struct Elf64ProgramHeader*)(((char *)boot_module->file_data) + boot_module->file_data->e_phoff); j < boot_module->file_data->e_phnum; j++, ph++) {
			if (ph->p_vaddr < reserved_start) reserved_start = ph->p_vaddr;
			if (ph->p_vaddr + ph->p_memsz > reserved_end) reserved_end = ph->p_vaddr + ph->p_memsz;
		}
		// Reserve the memory for the module:
		module_base = virtual_commit(boot_loader_allocator, NUM_OF_PAGES(reserved_end - reserved_start));
		if (module_base == NULL) return 0x1000;
		// Load the program headers to the memory:
		// Iterate over the program header tables:
		for (j = 0, ph = (struct Elf64ProgramHeader*)(((char *)boot_module->file_data) + boot_module->file_data->e_phoff); j < boot_module->file_data->e_phnum; j++, ph++) {
			ret = virtual_pages_alloc(module_base + ph->p_vaddr, NUM_OF_PAGES(ph->p_memsz), PAGE_ACCESS_RWX);
			if (ret == NULL) return 0x2000;
			memcpy(module_base + ph->p_vaddr, ((char*)boot_module->file_data) + ph->offset, ph->p_filesz);
			ASSERT(ph->p_filesz <= ph->p_memsz); // TODO: Handle this assert thing
			if (ph->p_filesz < ph->p_memsz) {
				memset(module_base + ph->p_vaddr + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);
			}
		}	

		// Get string table ptr:
		char * string_table = ((char*)boot_module->file_data) + ((struct Elf64SectionHeader *)(((char*)boot_module->file_data) + boot_module->file_data->e_shoff))->s_offset;

		// Handle exports:
		struct Elf64Symbol * symbol_entry;
		struct Elf64Symbol * symbol_table = (struct Elf64Symbol *) find_section_by_name(boot_module, string_strtab, &size);
		if (symbol_table == NULL) return 0x2800;
		for (symbol_entry = symbol_table; (char *)symbol_entry < ((char *)symbol_table) + size; symbol_entry++) {
			if (memcmp(string_table + symbol_entry->sym_name, export_prefix, sizeof(export_prefix))) continue; // skip non export symbol
			if (symbol_entry->sym_info >> 4 == 1) continue; // it is STB_GLOBAL - means that is import and not an export.
			ret = add_to_symbol_table(string_table + symbol_entry->sym_name, module_base + symbol_entry->sym_value);
			if (ret != 0) return 0x3000 + ret;
		}

		// Handle relocations:
		// There is nothing to do if there is no rel section
		if (find_section_by_type(boot_module, SHT_REL, &size)) return 0x4000; // Not supported such relocations.
	}

	// Now handle the imports:
	for (i = 0, boot_module = boot_modules; i < num_of_modules; i++, boot_module++) {
		struct Elf64Symbol * dynsym_table = (struct Elf64RelaStruct *) find_section_by_type(boot_module, SHT_DYNSYM, &size);
		struct Elf64Symbol * dynsym;
		struct Elf64SectionHeader * sh = ((char *)boot_module) + boot_module->file_data->e_shoff;
		char * dynstr;
		struct Elf64RelaStruct * rela_table = (struct Elf64RelaStruct *) find_section_by_type(boot_module, SHT_RELA, &size);
		size = size /= sizeof(struct Elf64RelaStruct); // num of structs.
		struct Elf64RelaStruct * rela;
		if (!rela) continue; // No imports to this module.
		if (count_sections_by_type(boot_module, SHT_RELA) > 1) return 0x5000; // Not supported more then one rela section.
		for (rela = rela_table; rela < rela_table + size; rela++) {
			if (rela->r_type != 7) return 0x6000;
			dynsym = dynsym_table + rela->r_index;
			dynstr = ((char*)boot_module) + (sh + dynsym->sym_shndx)->s_offset;
			Elf64_Addr symbol_addr = find_symbol(dynstr + dynsym->sym_name);
			if (symbol_addr == NULL) return 0x7000;
			*((Elf64_Addr *)(module_base + rela->r_addr)) = symbol_addr;
		}
	}
	return 0;
}
// The function returns pointer to the first section data with type 'type'. the size of the section will be in size_out.
// Return NULL if not found. size_out in that case will be set to 0.
void * find_section_by_type(struct STAGE0BootModule * boot_modules, s_type64_e type, int * size_out) {
	int sn = boot_modules->file_data->e_shnum;
	struct Elf64SectionHeader * sh = (struct Elf64SectionHeader *) (((char*)boot_modules->file_data) + boot_modules->file_data->e_shoff);
	for (int i = 0; i < sn; i++, sh++) {
		if (sh->s_type == type) {
			*size_out = sh->s_size;
			return (void *)(((char *)boot_modules->file_data) + sh->s_offset);
		}
	}
	*size_out = 0;
	return NULL;
}
// The function returns pointer to the first section data with name 'name'. the size of the section will be in size_out.
// Return NULL if not found. size_out in that case will be set to 0.
void * find_section_by_name(struct STAGE0BootModule * boot_modules, char * name, int * size_out) {
	int sn = boot_modules->file_data->e_shnum;
	struct Elf64SectionHeader * sh = (struct Elf64SectionHeader *) (((char*)boot_modules->file_data) + boot_modules->file_data->e_shoff);
	
	char * symtab = ((char *)boot_modules->file_data) + (sh + boot_modules->file_data->e_shtrndx)->s_offset;
	for (int i = 0; i < sn; i++, sh++) {
		
		if (strcmp(symtab + sh->s_name,name) == 0) {
			*size_out = sh->s_size;
			return (void *)(((char *)boot_modules->file_data) + sh->s_offset);
		}
	}
	*size_out = 0;
	return NULL;
}
// The function returns the number of sections data with type 'type'.
int count_sections_by_type(struct STAGE0BootModule * boot_modules, s_type64_e type) {
	int sn = boot_modules->file_data->e_shnum;
	int ret = 0;
	struct Elf64SectionHeader * sh = (struct Elf64SectionHeader *) (((char*)boot_modules->file_data) + boot_modules->file_data->e_shoff);
	for (int i = 0; i < sn; i++, sh++) {
		if (sh->s_type == type) ret++;
	}
	return ret;
}

/// Symbol table functions:
int add_to_symbol_table(char * sym_name, Elf64_Addr sym_addr) {
	return 0;
}

Elf64_Addr find_symbol(char * sym_name) {
	return 0x1212121212121212;
}

