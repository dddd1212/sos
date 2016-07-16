#include "loader.h"
#include "../QbjectManager/Qbject.h"

QResult load_driver(int8 * path)
{
	return QFail;
	QHandle qhandle = create_qbject(path, ACCESS_READ | ACCESS_WRITE);
	if (qhandle == NULL) return QFail;
	uint64 res_num_read = 0;
	uint64 total_read = 0;
	//QResult res = read_qbject(qhandle, uint8* buffer, uint64 position, uint64 num_of_bytes_to_read, uint64* res_num_read);
	
	return QSuccess;
}
typedef struct {
	char * file_name; // pointer to the module file name
	Elf64Header * file_data; // data of the eff file.
	unsigned int file_size; // num of pages the file need.
	//char * module_base; // pointer to the loaded module.
	//EntryPoint entry_point; // module entry point
	//int symbols_start_index;
} ModuleMetaData;

QResult load_driver_binary(uint8 * elf_binary, uint32 size, int8 * file_name)
{
	struct Elf64ProgramHeader * ph;
	int i, j;
	Elf64_Addr module_base;
	uint32 ret;
	int32 ret2;
	char * string_dynsym = ".dynsym";
	char * string_dynstr = ".dynstr";
	char * entry_point_name = "qkr_main";

	Elf64_Xword size = 0;
	// For now, we not handle dependecies, so we build just one element:
	ModuleMetaData module;
	module.file_name = file_name;
	module.file_data = (Elf64Header)


	// First - load the segments, handle exports, and reloacations
	for (i = 0, boot_module = boot_modules; i < num_of_modules; i++, boot_module++) {
		boot_module->symbols_start_index = kgd->bootloader_symbols.index;
		DBG_PRINTF2("Load the segments, handle exports and relocations of module %d (%s)", i, boot_modules[i].file_name); ENTER;
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
		module_base = (Elf64_Addr)virtual_commit(boot_loader_allocator, reserved_end - reserved_start, FALSE);
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
				memset((char*)(module_base - reserved_start + ph->p_vaddr + ph->p_filesz), 0, ph->p_memsz - ph->p_filesz);
			}
		}

		// Get string table ptr:
		char * string_table = (char *)find_section_by_name(boot_module, string_dynstr, &size);
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
				boot_module->entry_point = (EntryPoint)module_base + symbol_entry->sym_value;
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
		DBG_PRINTF2("Handle relocations (and imports) of module %d (%s)", i, boot_modules[i].file_name); ENTER;
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
				}
				else if (dynsym->sym_shndx == 0) { // We should find the symbol in the global symbols table.
					symbol_addr = find_symbol(kgd, dynstr + dynsym->sym_name);
					DBG_PRINTF2("    search symbol result: *%s = 0x%x", dynstr + dynsym->sym_name, symbol_addr); ENTER;
				}
				else { // The symbol is local and found in the dynsym section:
					symbol_addr = (Elf64_Addr)boot_module->module_base + dynsym->sym_value;
					DBG_PRINTF2("    handle rela: *%s = 0x%x", dynstr + dynsym->sym_name, symbol_addr); ENTER;
				}
				if (symbol_addr == NULL) {
					DBG_PRINTF1("    ERROR: could not resolve symbol! (%s)", dynstr + dynsym->sym_name); ENTER;
					return 0x7000;
				}
				if (rela->r_type == R_AMD64_64) {
					*((Elf64_Addr *)(boot_module->module_base + rela->r_addr)) += symbol_addr;
				}
				else {
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
			DBG_PRINTF2("No entry point to module %d (%s)", i, boot_modules[i].file_name); ENTER;
			continue;
		}
		DBG_PRINTF3("Call entry point of module %d (%s), 0x%x", i, boot_modules[i].file_name, boot_module->entry_point); ENTER;
		if ((ret2 = boot_module->entry_point(kgd)) != 0) {
			DBG_PRINTF("    ERROR: entry point return non zero!"); ENTER;
			return i * 0x10000000 + ret2;
		}
	}
	// should not reach here!
	DBG_PRINTF("ERROR: should not reach here!"); ENTER;
	return 0x123123;
	return QSuccess;
}
