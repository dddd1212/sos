#include "Qube.h"
#include "long_mode.h"
#include "hd.h"
#include "mem.h"
#include "loader.h"
#include "libc.h"

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
	kgd = (struct KernelGlobalData *) mem_alloc(&allocator, alloc_pages << 12, FALSE);
	if (kgd == NULL || ret == NULL) {
		STAGE0_suicide(0x5000);
	}
	memset((char *)kgd, 0, sizeof(struct KernelGlobalData) + sizeof(ModulesList));

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

	// init the symbol table:
	kgd->bootloader_symbols.symbols = (struct Symbol *)mem_alloc(&allocator, sizeof(struct Symbol)*MAX_PRIMITIVE_SYMBOLS + MAX_NAME_STORAGE,FALSE);
	kgd->bootloader_symbols.names_storage = ((char *)kgd->bootloader_symbols.symbols) + sizeof(struct Symbol)*MAX_PRIMITIVE_SYMBOLS;
	kgd->bootloader_symbols.index = 0;
	kgd->bootloader_symbols.names_storage_index = 0;

	// load the modules:
	int ret2 = load_modules(kgd, boot_modules, &allocator, num_of_lines);
	if (ret2 != 0) STAGE0_suicide(ret2);

	// Should not reach here!
	STAGE0_suicide(0xffffffff);
}

void STAGE0_suicide(int error) {
	int * s = 0;
	*s = 0;
}





