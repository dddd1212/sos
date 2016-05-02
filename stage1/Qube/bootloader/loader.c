
#include "../Common/Qube.h"
#include "hd.h"
#include "mem.h"
#include "loader.h"
#include "libc.h"
#include "screen.h"
// kgd - pointer to KernelGlobalData\
// boot_modules - ptr to array of STAGE0BootModules
// num_of_modules - boot_modules entries
int load_modules_and_run_kernel(KernelGlobalData * kgd, struct STAGE0BootModule * boot_modules, BootLoaderAllocator * boot_loader_allocator, int num_of_modules) {
	ScreenHandle * screen_ptr = kgd->boot_info->scr; // This name is need to make the DBG_PRINT macro works.
	DBG_PRINT("load_modules_and_run_kernel called!");
	struct STAGE0BootModule * boot_module;
	struct Elf64ProgramHeader * ph;
	int i, j;
	Elf64_Addr module_base;
	uint32 ret;
	int ret2;
	char string_strtab[] = { '.','s','t','r','t','a','b','\x00' };
	char export_prefix[] = { 'E','X','P','_' };
	char entry_point_prefix[] = { 'E','N','P','_' };
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
		module_base = (Elf64_Addr) virtual_commit(boot_loader_allocator, reserved_end - reserved_start, FALSE);
		if (module_base == NULL) return 0x1000;
		// Load the program headers to the memory:
		// Iterate over the program header tables:
		for (j = 0, ph = (struct Elf64ProgramHeader*)(((char *)boot_module->file_data) + boot_module->file_data->e_phoff); j < boot_module->file_data->e_phnum; j++, ph++) {
			ret = alloc_committed(boot_loader_allocator, ph->p_memsz, (void*)(module_base - reserved_start + ph->p_vaddr));
			if (ret == NULL) return 0x2000;
			memcpy((char*)(module_base - reserved_start + ph->p_vaddr), ((char*)boot_module->file_data) + ph->offset, ph->p_filesz);
			// CR: Ha?
			ASSERT(ph->p_filesz <= ph->p_memsz); // TODO: Handle this assert thing
			if (ph->p_filesz < ph->p_memsz) {
				memset((char*) (module_base - reserved_start + ph->p_vaddr + ph->p_filesz), 0, ph->p_memsz - ph->p_filesz);
			}
		}	

		// Get string table ptr:
		char * string_table = ((char*)boot_module->file_data) + ((struct Elf64SectionHeader *)(((char*)boot_module->file_data) + boot_module->file_data->e_shoff))[boot_module->file_data->e_shstrndx].s_offset;

		// Handle exports:
		struct Elf64Symbol * symbol_entry;
		struct Elf64Symbol * symbol_table = (struct Elf64Symbol *) find_section_by_name(boot_module, string_strtab, &size);
		if (symbol_table == NULL) return 0x2800;
		for (symbol_entry = symbol_table; (char *)symbol_entry < ((char *)symbol_table) + size; symbol_entry++) {
			if (memcmp(string_table + symbol_entry->sym_name, entry_point_prefix, sizeof(entry_point_prefix)) == 0) {
				if (boot_module->entry_point) {
					return 0x2500; // to many entry points!
				}
				boot_module->entry_point = (EntryPoint) module_base + symbol_entry->sym_value;
			}
			if (memcmp(string_table + symbol_entry->sym_name, export_prefix, sizeof(export_prefix))) continue; // skip non export symbol
			if (symbol_entry->sym_info >> 4 == 1) continue; // it is STB_GLOBAL - means that is import and not an export.
			ret2 = add_to_symbol_table(kgd, string_table + symbol_entry->sym_name, module_base + symbol_entry->sym_value);
			if (ret2 != 0) return 0x3000 + ret2;
		}

		// Handle relocations:
		// There is nothing to do if there is no rel section
		if (find_section_by_type(boot_module, SHT_REL, &size)) return 0x4000; // Not supported such relocations.
	}

	// Now handle the imports:
	for (i = 0, boot_module = boot_modules; i < num_of_modules; i++, boot_module++) {
		struct Elf64Symbol * dynsym_table = (struct Elf64Symbol *) find_section_by_type(boot_module, SHT_DYNSYM, &size);
		struct Elf64Symbol * dynsym;
		struct Elf64SectionHeader * sh = (struct Elf64SectionHeader *) (((char *)boot_module) + boot_module->file_data->e_shoff);
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
			Elf64_Addr symbol_addr = find_symbol(kgd, dynstr + dynsym->sym_name);
			if (symbol_addr == NULL) return 0x7000;
			*((Elf64_Addr *)(module_base + rela->r_addr)) = symbol_addr;
		}
	}

	// Finally, call to the entry points:
	for (i = 0, boot_module = boot_modules; i < num_of_modules; i++, boot_module++) {
		if (ret2 = boot_module->entry_point(kgd) != 0) return i*0x10000000 + ret2;
	}
	// should not reach here!
	return 0;
}
// The function returns pointer to the first section data with type 'type'. the size of the section will be in size_out.
// Return NULL if not found. size_out in that case will be set to 0.
void * find_section_by_type(struct STAGE0BootModule * boot_modules, s_type64_e type, Elf64_Xword * size_out) {
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
void * find_section_by_name(struct STAGE0BootModule * boot_modules, char * name, Elf64_Xword * size_out) {
	int sn = boot_modules->file_data->e_shnum;
	struct Elf64SectionHeader * sh = (struct Elf64SectionHeader *) (((char*)boot_modules->file_data) + boot_modules->file_data->e_shoff);
	
	char * symtab = ((char *)boot_modules->file_data) + (sh + boot_modules->file_data->e_shstrndx)->s_offset;
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
// return: 0 - success
//         1 - failure
int add_to_symbol_table(KernelGlobalData * kgd, char * sym_name, Elf64_Addr sym_addr) {
	int symlen = strlen(sym_name) + 1; // include the null.
	// Check for memory:
	if (kgd->bootloader_symbols.index >= MAX_PRIMITIVE_SYMBOLS) return 1;
	if (kgd->bootloader_symbols.names_storage_index + symlen > MAX_NAME_STORAGE) return 1;

	// copy the name to the name storage:
	strcpy(kgd->bootloader_symbols.names_storage + kgd->bootloader_symbols.names_storage_index, sym_name);

	// fill the symbol:
	(kgd->bootloader_symbols.symbols + kgd->bootloader_symbols.index)->addr = sym_addr;
	(kgd->bootloader_symbols.symbols + kgd->bootloader_symbols.index)->name = kgd->bootloader_symbols.names_storage + kgd->bootloader_symbols.names_storage_index;
	
	// increment the indices:
	kgd->bootloader_symbols.names_storage_index += symlen;
	kgd->bootloader_symbols.index++;
	return 0;
}

Elf64_Addr find_symbol(KernelGlobalData * kgd, char * sym_name) {
	int max = kgd->bootloader_symbols.index;
	struct Symbol * cur = kgd->bootloader_symbols.symbols;
	for (int i = 0; i < max; i++, cur++) {
		if (strcmp(sym_name, cur->name) == 0) return cur->addr;

	}
	return NULL;
}

