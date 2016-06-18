
#include "../Common/Qube.h"
#include "hd.h"
#include "mem.h"
#include "loader.h"
#include "libc.h"
#include "screen.h"

void hack_for_gdb(char * module_file_name, Elf64_Addr entry) {
	return;
}
void hack_for_gdb2() { 
	return;
}
// kgd - pointer to KernelGlobalData
// boot_modules - ptr to array of STAGE0BootModules
// num_of_modules - boot_modules entries
int load_modules_and_run_kernel(KernelGlobalData * kgd, struct STAGE0BootModule * boot_modules, BootLoaderAllocator * boot_loader_allocator, int num_of_modules) {
	ScreenHandle * screen_ptr = kgd->boot_info->scr; // This name is need to make the DBG_PRINT macro works.
	DBG_PRINT("load_modules_and_run_kernel called!"); ENTER;
	struct STAGE0BootModule * boot_module;
	struct Elf64ProgramHeader * ph;
	int i, j;
	Elf64_Addr module_base;
	uint32 ret;
	int ret2;
	//char string_strtab[] = {".strtab" };
	char string_dynsym[] = {".dynsym" };
	char string_dynstr[] = {".dynstr" };
	//char string_rela_plt[] = { ".rela.plt" };
	//char string_rela_dyn[] = { ".rela.dyn" };
	char entry_point_name[] = { "qkr_main" };

	Elf64_Xword size = 0;
	// First - load the segments, handle exports, and reloacations
	for (i = 0, boot_module = boot_modules; i < num_of_modules; i++, boot_module++) {
		boot_module->symbols_start_index = kgd->bootloader_symbols.index;
		DBG_PRINTF1("Load the segments, handle exports and relocations of module %d", i); ENTER;
		// Calc the size of virtual memory we need to reserve:
		// Iterate over the program header tables:
		Elf64_Xword reserved_start = 0xffffffffffffffff;
		Elf64_Xword reserved_end = 0;
		for (j = 0, ph = (struct Elf64ProgramHeader*)(((char *)boot_module->file_data) + boot_module->file_data->e_phoff); j < boot_module->file_data->e_phnum; j++, ph++) {
			if (ph->p_vaddr < reserved_start) reserved_start = ph->p_vaddr;
			if (ph->p_vaddr + ph->p_memsz > reserved_end) reserved_end = ph->p_vaddr + ph->p_memsz;
		}
		DBG_PRINTF1("    virtual memory size that we need to reserve: 0x%x", reserved_end - reserved_start); ENTER;
		// Reserve the memory for the module:
		module_base = (Elf64_Addr) virtual_commit(boot_loader_allocator, reserved_end - reserved_start, FALSE);
		boot_module->module_base = (char*)module_base;
		DBG_PRINTF1("    Module_base: 0x%x", module_base); ENTER;
		if (module_base == NULL) {
			DBG_PRINTF("    ERROR: Module_base is null!"); ENTER;
			return 0x1000;
		}
		// Load the program headers to the memory:
		// Iterate over the program header tables:
		for (j = 0, ph = (struct Elf64ProgramHeader*)(((char *)boot_module->file_data) + boot_module->file_data->e_phoff); j < boot_module->file_data->e_phnum; j++, ph++) {
			DBG_PRINTF1("    Load segment #%d", j); ENTER;
			uint64 seg_start = (uint64)(module_base - reserved_start + ph->p_vaddr);
			// Now align the start to page:
			int pad_size = (seg_start%PAGE_SIZE);
			ret = alloc_committed(boot_loader_allocator, ph->p_memsz + pad_size, (void*)(seg_start - pad_size));
			if (ret != QSuccess) {
				DBG_PRINTF("    ERROR: alloc failed!"); ENTER;
				return 0x2000;
			}
			if (pad_size) {
				memset((char*)(module_base - reserved_start + ph->p_vaddr - pad_size), 0, pad_size);
			}
			memcpy((char*)(module_base - reserved_start + ph->p_vaddr), ((char*)boot_module->file_data) + ph->offset, ph->p_filesz);
			// CR: Ha?
			// CR ANSWER - GILAD: what? by the RFC the assert should pass.
			ASSERT(ph->p_filesz <= ph->p_memsz);
			if (ph->p_filesz < ph->p_memsz) {
				memset((char*) (module_base - reserved_start + ph->p_vaddr + ph->p_filesz), 0, ph->p_memsz - ph->p_filesz);
			}
		}	

		// Get string table ptr:
		char * string_table = (char *) find_section_by_name(boot_module, string_dynstr, &size);
		DBG_PRINTF1("    file string table address: 0x%x", string_table); ENTER;
		// Handle exports:
		struct Elf64Symbol * symbol_entry;
		struct Elf64Symbol * symbol_table = (struct Elf64Symbol *) find_section_by_name(boot_module, string_dynsym, &size);
		DBG_PRINTF1("    symbol table address: 0x%x", symbol_table); ENTER;
		if (symbol_table == NULL) {
			DBG_PRINTF("    ERROR: could no found dynsym section!"); ENTER;
			return 0x2800;
		}
		for (symbol_entry = symbol_table; (char *)symbol_entry < ((char *)symbol_table) + size; symbol_entry++) {
			if (memcmp(string_table + symbol_entry->sym_name, entry_point_name, sizeof(entry_point_name)) == 0) {
				if (boot_module->entry_point) {
					DBG_PRINTF("    ERROR: too many entry points!"); ENTER;
					return 0x2500; // to many entry points!
				}
				boot_module->entry_point = (EntryPoint) module_base + symbol_entry->sym_value;
				DBG_PRINTF1("    Found entry point: 0x%x", boot_module->entry_point); ENTER;
			}
			if (symbol_entry->sym_info_bind != STB_GLOBAL ||
				symbol_entry->sym_info_type == STT_NOTYPE ||
				symbol_entry->sym_shndx == 0) continue; // skip non export symbol
			DBG_PRINTF2("    Found symbol: '%s' at address 0x%x", string_table + symbol_entry->sym_name, module_base + symbol_entry->sym_value); ENTER;
			ret2 = add_to_symbol_table(kgd, string_table + symbol_entry->sym_name, module_base + symbol_entry->sym_value);
			if (ret2 != 0) {
				DBG_PRINTF("    ERROR: error while adding to symbol table!"); ENTER;
				return 0x3000 + ret2;
			}
		}

		// Handle rel sections:
		// There is nothing to do if there is no rel section
		if (find_section_by_type(boot_module, SHT_REL, NULL, &size)) {
			DBG_PRINTF("    ERROR: we not support SHT_REL sections!"); ENTER;
			return 0x4000; // Not supported such relocations.
		}
	}

	// Now handle the imports and the rela sections:
	//int num_of_rela_sections = 0;
	for (i = 0, boot_module = boot_modules; i < num_of_modules; i++, boot_module++) {
		DBG_PRINTF1("Handle relocations (and imports) of module %d", i); ENTER;
		int start_index = 0;
		//struct Elf64SectionHeader * sh = (struct Elf64SectionHeader *) (((char *)boot_module) + boot_module->file_data->e_shoff);
		struct Elf64Symbol * dynsym_table = (struct Elf64Symbol *) find_section_by_type(boot_module, SHT_DYNSYM, NULL, &size);
		struct Elf64Symbol * dynsym;
		struct Elf64RelaStruct * rela_table;
		char * dynstr = (char *)find_section_by_name(boot_module, string_dynstr, NULL);
		while (1) { // Iterate over SHT_RELA sections:
			rela_table = (struct Elf64RelaStruct *) find_section_by_type(boot_module, SHT_RELA, &start_index, &size);
			if (!rela_table) break; // finish iterate.
			size = size / sizeof(struct Elf64RelaStruct); // num of structs.
			struct Elf64RelaStruct * rela;
			for (rela = rela_table; rela < rela_table + size; rela++) {
				if (rela->r_type != R_AMD64_GLOB_DAT && rela->r_type != R_AMD64_JUMP_SLOT && rela->r_type != R_AMD64_64 && rela->r_type != R_AMD64_RELATIVE) {
					DBG_PRINTF1("    ERROR: unsupported rela->r_type (=%d)!", rela->r_type); ENTER;
					return 0x6000;
				}
				dynsym = dynsym_table + rela->r_index;	
				
				Elf64_Addr symbol_addr = NULL;
				if (rela->r_type == R_AMD64_RELATIVE) { // The symbol is relative. we should not search it anyware.
					symbol_addr = (uint64)boot_module->module_base + rela->r_addend;
					DBG_PRINTF2("    handle relative rela: *0x%x = 0x%x", boot_module->module_base + rela->r_addr, symbol_addr); ENTER;
				} else if (dynsym->sym_shndx == 0) { // We should find the symbol in the global symbols table.
					symbol_addr = find_symbol(kgd, dynstr + dynsym->sym_name);
					DBG_PRINTF2("    search symbol result: *%s = 0x%x", dynstr + dynsym->sym_name, symbol_addr); ENTER;
				} else { // The symbol is local and found in the dynsym section:
					symbol_addr = (Elf64_Addr)boot_module->module_base + dynsym->sym_value;
					DBG_PRINTF2("    handle rela: *%s = 0x%x", dynstr + dynsym->sym_name, symbol_addr); ENTER;
				}
				if (symbol_addr == NULL) {
					DBG_PRINTF1("    ERROR: could not resolve symbol! (%s)", dynstr + dynsym->sym_name); ENTER;
					return 0x7000;
				}
				if (rela->r_type == R_AMD64_64) {
					*((Elf64_Addr *)(boot_module->module_base + rela->r_addr)) += symbol_addr;
				} else {
					*((Elf64_Addr *)(boot_module->module_base + rela->r_addr)) = symbol_addr;
				}
			}
		}
	}
#ifdef DEBUG
	for (i = 0, boot_module = boot_modules; i < num_of_modules; i++, boot_module++) {
		char * module_name = boot_module->file_name;
		Elf64_Addr addr_to_gdb = (Elf64_Addr)boot_module->module_base + boot_module->file_data->e_entry;
		hack_for_gdb(module_name, addr_to_gdb);
		DBG_PRINTF2("Symbol start to module %s: 0x%x", module_name, addr_to_gdb); ENTER;
	}
#endif
	hack_for_gdb2();
	set_boot_info(boot_loader_allocator, kgd->boot_info);
	// Finally, call to the entry points:
	for (i = 0, boot_module = boot_modules; i < num_of_modules; i++, boot_module++) {
		
		if (!boot_module->entry_point) {
			DBG_PRINTF1("No entry point to module %d", i); ENTER;
			continue;
		}
		DBG_PRINTF1("Call entry point of module %d", i); ENTER;
		if ((ret2 = boot_module->entry_point(kgd)) != 0) {
			DBG_PRINTF("    ERROR: entry point return non zero!"); ENTER;
			return i * 0x10000000 + ret2;
		}
	}
	// should not reach here!
	DBG_PRINTF("ERROR: should not reach here!"); ENTER;
	return 0x123123;
}
// The function returns pointer to the first section data with type 'type'. the size of the section will be in size_out.
// The function starts looking from the section index '*start_index'. if start_index is null, then starts looking from 0.
// When returning not NULL, start_index will be fill with the found section index+1.
// Return NULL if not found. size_out in that case will be set to 0.
void * find_section_by_type(struct STAGE0BootModule * boot_modules, s_type64_e type, int * start_index, Elf64_Xword * size_out) {
	int sn = boot_modules->file_data->e_shnum;
	struct Elf64SectionHeader * sh = (struct Elf64SectionHeader *) (((char*)boot_modules->file_data) + boot_modules->file_data->e_shoff);
	int i = 0;
	if (start_index) {
		i = *start_index;
		sh += i;
	}
	for (; i < sn; i++, sh++) {
		if (sh->s_type == type) {
			*size_out = sh->s_size;
			*start_index = i + 1;
			return (void *)(((char *)boot_modules->file_data) + sh->s_offset);
		}
	}
	*start_index = 0;
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
			if (size_out) *size_out = sh->s_size;
			return (void *)(((char *)boot_modules->file_data) + sh->s_offset);
		}
	}
	if (size_out) *size_out = 0;
	return NULL;
}
/*
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
*/
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

