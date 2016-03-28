#include "long_mode.h"
#include "hd.h"
#include "mem.h"
// This file initialize the very first things in the kernel:
// 1. init the KernelGlobalData struct.
//    a. Init the symbols table. (The symbols table will be right after 
// 2. go over a list of blobs that contains the stage1-init-chain-drivers and load them.
//
//
// The file is compiled as image and it found in the memory right before the KernelGlobalData struct.

struct STAGE0BootModules {
	char * file_name; // pointer to the module file name
	unsigned int file_pages; // num of pages the file need.
	void * file_data; // data of the eff file.
	void * module_base; // pointer to the loaded module.
	void * entry_point; // module entry point
};

// This is the function that the bootloader is.
void main(void * my_address) {
	char boot_txt_data[BOOT_TXT_FILE_MAX_SIZE];
	struct STAGE0BootModules boot_modules[MAX_BOOT_MODULES];
	char * boot_txt_end;
	
	char boot_txt_file_name[] = { 'b','o','o','t','.','t','x','t','\x00' };
	unsigned int boot_txt_file_size;

	int alloc_pages = NUM_OF_PAGES(sizeof(struct KernelGlobalData)) + // the kgd will be the first in the memory
		NUM_OF_PAGES(sizeof(ModulesList)); // The modules list will be just after the kgd.
	struct KernelGlobalData * kgd = NULL;
	void * modules_addr = NULL;
	void * ret = NULL;
	int i, temp, line, num_of_lines;
	
	// First, calculate how many bytes we need to allocate for the kgd (KernelGlobalData), for the symbol table, and for the boot modules:
	boot_txt_file_size = STAGE0_get_file_size(boot_txt_file_name);
	if (boot_txt_file_size >= BOOT_TXT_FILE_MAX_SIZE) {
		STAGE0_suicide(0x1000);
	}
	if (boot_txt_file_size == 0) {
		STAGE0_suicide(0x2000);
	}

	STAGE0_read_file(boot_txt_file_name, boot_txt_data);
	boot_txt_data[boot_txt_file_size] = '\x00';

	i = 0;
	line = 0;
	while (boot_txt_data[i] != '\x00') {
		if (line >= MAX_BOOT_MODULES) {
			STAGE0_suicide(0x3000);
		}
		boot_modules[line].file_name = &boot_txt_data[i]; // File
		line++;
		while (boot_txt_data[i] != '\n' || boot_txt_data[i] != '\x00') i++;
		if (boot_txt_data[i] == '\x00') { // last line and its ends with no \n
			break;
		}
		boot_txt_data[i] = '\x00';
		i++;

	}
	num_of_lines = line;

	// get alloc size:
	for (line = 0; line < num_of_lines; line++) {
		temp = STAGE0_get_file_size(boot_modules[line].file_name);
		boot_modules[line].file_pages = NUM_OF_PAGES(temp);
		if (temp == -1) STAGE0_suicide(0x4000); // error reading the file.
		alloc_pages += temp;
	}

	// Commit the wanted pages for the files:
	kgd = (struct KernelGlobalData *)STAGE0_virtual_commit(alloc_pages);
	ret = STAGE0_virtual_pages_alloc(kgd, alloc_pages, PAGE_ACCESS_RW);
	if (kgd == NULL || ret == NULL) {
		STAGE0_suicide(0x5000);
	}
	STAGE0_memset(kgd, 0, sizeof(struct KernelGlobalData) + sizeof(ModulesList));

	// init the KernelGLobalData and the modules array.
	kgd->modules = (ModulesList *)(kgd + 1); // points after the kgd.

											 // point to the start of the modules files
	modules_addr = (void*)(kgd->modules + 1); // points after the modules.

											  // Now we can read the boot-modules to the memory:
	for (line = 0; line < num_of_lines; line++) {
		// read
		STAGE0_read_file(boot_modules[line].file_name, modules_addr);

		// points to the data:
		boot_modules[line].file_data = modules_addr;

		// move to the next address
		modules_addr = (int)modules_addr + PAGE_SIZE * boot_modules[line].file_pages;
	}

	// Now its time to load the file into the memory:
	// First, we load the segments, handle relocation and export symbols.
	/* TODO
	int modules_list_index = 0;
	for (line = 0; line < num_of_lines; line++) {
		int i;
		EffSegment * seg;
		EffRelocation * rel;
		Eff * eff;
		void * ret;

		eff = boot_modules[line]->file_data;
		// Sanity
		if (eff->magic != EFF_MAGIC) {
			STAGE0_suicide(0x100000);
		}

		// calculate how many memory we need to commit:
		seg = eff + eff->segments_offset;
		memory_to_commit = 0;
		for (i = 0; i < eff->num_of_segments; i++, seg += sizeof(EffSegment)) {
			if (seg->virtual_address + seg->size > memory_to_commit) {
				memory_to_commit = seg->virtual_address + seg->size;
			}
		}

		// Commit memory for this module:
		void * module_base = STAGE0_virtual_commit(NUM_OF_PAGES(memory_to_commit));

		// Copy segments to the memory
		seg = eff + eff->segments_offset;
		for (i = 0; i < eff->num_of_segments; i++, seg += sizeof(EffSegment);) {
			if (seg->type & EffSegmentType_BYTES) {
				ret = STAGE0_virtual_pages_alloc(module_base + seg->virtual_address, NUM_OF_PAGES(seg->size), PAGE_ACCES_RWX);
				if (ret == NULL) {
					STAGE0_suicide(0x101500);
				}
				STAGE0_memcpy(module_base + seg->virtual_address, eff + seg->offset_in_file, seg->size);
			}
			else if (seg->type & EffSegmentType_BSS) {
				ret = STAGE0_virtual_pages_alloc(module_base + seg->virtual_address, NUM_OF_PAGES(seg->size), PAGE_ACCES_RW);
				if (ret == NULL) {
					STAGE0_suicide(0x101600);
				}
				STAGE0_memset(module_base + seg->virtual_address, '\x00', seg->size);
			}
			else {
				STAGE0_suicide(0x101000);
			}
		}

		// Relocating
		rel = eff + eff->relocations_offset;
		for (i = 0; i < eff->num_of_relocations; i++) {
			*(module_base + rel->virtual_address_offset) += module_base;
		}

		// Exports - Just register the module in the modules list:
		kgd->modules[line] = module_base;
		boot_modules[line].module_base = module_base;
		boot_modules[line].entry_point = module_base + eff + eff->entry_point_offset
	}
	
	// Then, we handle import symbols:
	for (line = 0; line < num_of_lines; line++) {
		eff = boot_modules[line]->file_data;
		module_base = boot_modules[line].module_base
			imp = eff + eff->imports_offset;

		for (i = 0; i < eff->num_of_imports; i++) {
			if (*(module_base + imp->virtual_address_offset) = STAGE0_find_symbol(kgd, imp->name_offset) == NULL) {
				STAGE0_suicide(0x102000);
			}
		}
	}
	*/
	// Finally, call to each module callback:
	// IMPORTANT: The boot modules need to be aware that some of thay import functions may be in modules that not yet called to their entry-point.
	for (line = 0; line < num_of_lines; line++) {
		//TODO
		//boot_modules[line].entry_point();
	}
	// Should not reach here!
	STAGE0_suicide(0xffffffff);
}

void * STAGE0_find_symbol(struct KernelGlobalData * kgd, char * symbol_name) {
	/*
	for (int i = 0; i < MAX_LOADED_MODULES; i++) {
		void * base = (void*)kgd->modules[i];
		EffExport * exp = base + ((Eff)(base))->exports_offset_when_loaded;
		for (int j = 0; j < kgd->modules[i]->num_of_exports; j++, exp++) {
			if (STAGE0_memcmp(base + exp->name_offset_when_loaded, symbol_name)) return base + exp->target_address_offset_when_loaded;
		}
	}
	*/
	// TODO
	return NULL;
}
// We include the files to make it compile like a one pic-code






void STAGE0_suicide(int error) {
	int * s = 0;
	*s = 0;
}
void STAGE0_memset(void * addr, char c, int count) {
	// TODO
	return;
}





