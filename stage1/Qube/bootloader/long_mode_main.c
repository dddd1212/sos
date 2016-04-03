#include "long_mode.h"
#include "hd.h"
#include "mem.h"
#include "loader.h"
// This file initialize the very first things in the kernel:
// 1. init the KernelGlobalData struct.
//    a. Init the symbols table. (The symbols table will be right after 
// 2. go over a list of blobs that contains the stage1-init-chain-drivers and load them.
//
//
// The file is compiled as image and it found in the memory right before the KernelGlobalData struct.

// This is the function that the bootloader is.
void _start(void * my_address) {
    BootLoaderAllocator allocator;
	hdDesc hd_desc;
	init_allocator(&allocator);
	init_hd(&allocator, &hd_desc); 
    
	char boot_txt_data[BOOT_TXT_FILE_MAX_SIZE];
	struct STAGE0BootModule boot_modules[MAX_BOOT_MODULES];
	char * boot_txt_end;
	
	char boot_txt_file_name[] = { 'B','O','O','T','.','T','X','T','\x00' };
	unsigned int boot_txt_file_size;

	int alloc_pages = NUM_OF_PAGES(sizeof(struct KernelGlobalData)) + // the kgd will be the first in the memory
		NUM_OF_PAGES(sizeof(ModulesList)); // The modules list will be just after the kgd.
	struct KernelGlobalData * kgd = NULL;
	void * modules_addr = NULL;
	void * ret = NULL;
	int i, temp, line, num_of_lines;
	
	// First, calculate how many bytes we need to allocate for the kgd (KernelGlobalData), for the symbol table, and for the boot modules:
	boot_txt_file_size = get_file_size(&hd_desc,boot_txt_file_name);
	if (boot_txt_file_size >= BOOT_TXT_FILE_MAX_SIZE) {
		STAGE0_suicide(0x1000);
	}
	if (boot_txt_file_size == 0) {
		STAGE0_suicide(0x2000);
	}

	read_file(&hd_desc, boot_txt_file_name, boot_txt_data);
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
		temp = get_file_size(&hd_desc, boot_modules[line].file_name);
		boot_modules[line].file_pages = NUM_OF_PAGES(temp);
		if (temp == -1) STAGE0_suicide(0x4000); // error reading the file.
		alloc_pages += temp;
	}

	// Commit the wanted pages for the files:
	//kgd = (struct KernelGlobalData *)virtual_commit(&allocator, alloc_pages);
	//ret = virtual_pages_alloc(&allocator, alloc_pages, PAGE_ACCESS_RW);
	kgd = (struct KernelGlobalData *) mem_alloc(&allocator, alloc_pages << 12, FALSE);
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
		read_file(&hd_desc, boot_modules[line].file_name, modules_addr);

		// points to the data:
		boot_modules[line].file_data = modules_addr;

		// move to the next address
		modules_addr = (int8*)modules_addr + PAGE_SIZE * boot_modules[line].file_pages;
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





