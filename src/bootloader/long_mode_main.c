#include "../Common/Qube.h"
#include "long_mode.h"
#include "hd.h"
#include "mem.h"
#include "loader.h"
#include "libc.h"
#include "screen.h"

// This file initialize the very first things in the kernel:
// 1. init the KernelGlobalData struct.
//    a. Init the symbols table. (The symbols table will be right after 
// 2. go over a list of blobs that contains the stage1-init-chain-drivers and load them.
//
//
// The file is compiled as image and it found in the memory right before the KernelGlobalData struct.

// This is the function that the bootloader is.
void _start() {
    BootLoaderAllocator allocator;
	ScreenHandle scr;
	ScreenHandle * screen_ptr = &scr; // This is just for the use of the DBG_PRINTx macro.
									  // TODO: remove it if we not in debug mode.
	hdDesc hd_desc;
	// CR: Gilad - check the return values
	init_allocator(&allocator);
	if (QFail == init_hd(&allocator, &hd_desc)) {
		STAGE0_suicide(NULL, 0x1000);
	}

	char boot_txt_data[BOOT_TXT_FILE_MAX_SIZE];
	struct STAGE0BootModule boot_modules[MAX_BOOT_MODULES];
	//char * boot_txt_end;
	char boot_txt_file_name[] = { 'B','O','O','T','.','T','X','T','\x00' };
	unsigned int boot_txt_file_size;

	int32 alloc_bytes = ALIGN_UP(sizeof(KernelGlobalData)) + // the kgd will be the first in the memory
		ALIGN_UP(sizeof(ModulesList)); // The modules list will be just after the kgd.

	KernelGlobalData * kgd = NULL;
	// Commit the wanted pages for the files:
	// CR: there is one nonvolatile alloc for kgd + modules-list + modules-data, but isn't the module-data should be volatile?
	// CR COMPLETE: Gilad - Done.
	kgd = (KernelGlobalData *)mem_alloc(&allocator, alloc_bytes, FALSE);
	kgd->boot_info = (BootInfo *)mem_alloc(&allocator, sizeof(BootInfo), FALSE);
	alloc_bytes = 0;
	// CR: ret set to NULL...
	if (kgd == NULL) {
		STAGE0_suicide(NULL, 0x5000);
	}
	// Initialize the kgd:
	memset((char *)kgd, 0, ALIGN_UP(sizeof(KernelGlobalData)) + ALIGN_UP(sizeof(ModulesList)));
	// init the KernelGLobalData and the modules array.
	kgd->modules = (ModulesList *)(ALIGN_UP(kgd + 1)); // points after the kgd.

													   // point to the start of the modules files

	// map the first MB of pysical memory, and store pointer of it in kgd.
	kgd->first_MB = map_first_MB(&allocator);

	// Init the screen:
	INIT_SCREEN(&scr, kgd->first_MB);
	kgd->boot_info->scr = &scr;
	PUTS(&scr, "Screen init complete successfuly!");

	//acpi_test(&scr, kgd);
	

	void * modules_addr = NULL;
	int32 i, temp, line, num_of_lines;
	
	// First, calculate how many bytes we need to allocate for the kgd (KernelGlobalData), for the symbol table, and for the boot modules:
	//DBG_PRINTF("Calculate how many bytes we need to allocate the boot modules.."); ENTER;
	boot_txt_file_size = get_file_size(&hd_desc, boot_txt_file_name);
	if (boot_txt_file_size == 0xffffffff) {
		DBG_PRINTF("Cannot find boot file!"); ENTER;
		STAGE0_suicide(&scr, 0x1080);
	}
	if (boot_txt_file_size >= BOOT_TXT_FILE_MAX_SIZE) {
		DBG_PRINTF2("BOOT_TXT_FILE too big! (%d > %d)", boot_txt_file_size, BOOT_TXT_FILE_MAX_SIZE); ENTER;
		STAGE0_suicide(&scr, 0x1100);
	}
 	if (boot_txt_file_size == 0) {
		//DBG_PRINTF2("BOOT_TXT_FILE size is zero!", boot_txt_file_size, BOOT_TXT_FILE_MAX_SIZE); ENTER;
		STAGE0_suicide(&scr, 0x2000);
	}
	// TODO: check return value
	read_file(&hd_desc, boot_txt_file_name, boot_txt_data);
	boot_txt_data[boot_txt_file_size] = '\x00';

	i = 0;
	line = 0;
	while (boot_txt_data[i] != '\x00') {
		if (line >= MAX_BOOT_MODULES) {
			STAGE0_suicide(&scr, 0x3000);
		}
		boot_modules[line].file_name = &boot_txt_data[i]; // File
		
		boot_modules[line].entry_point = 0;
		line++;
		while ((boot_txt_data[i] != '\n') && (boot_txt_data[i] != '\x00')) i++;
		if (boot_txt_data[i] == '\x00') { // last line and it ends with no \n
			DBG_PRINTF2("module %d name: %s...", line - 1, boot_modules[line - 1].file_name); ENTER;
			break;
		}
		boot_txt_data[i] = '\x00';
		if (i > 0 && boot_txt_data[i-1] == '\x0d') {
			boot_txt_data[i - 1] = '\x00';
		}
		i++;
		DBG_PRINTF2("module %d name: %s...", line - 1, boot_modules[line - 1].file_name); ENTER;
	}
	num_of_lines = line;
	
	// get alloc size:
	// CR Dror: dont we want to alloc each module on page boundary? looks like the next code assume this, and it should be "alloc_bytes += PAGES_ROUND_UP(temp)" or something.
	// CR-COMPLETE Gilad.
	for (line = 0; line < num_of_lines; line++) {
		temp = get_file_size(&hd_desc, boot_modules[line].file_name);
		boot_modules[line].file_pages = NUM_OF_PAGES(temp);
		if (temp == -1) STAGE0_suicide(&scr, 0x4000 + line); // error reading the file.
		alloc_bytes += boot_modules[line].file_pages * PAGE_SIZE;
		DBG_PRINTF3("module %d size: 0x%x. Allocate %d pages.", line, temp, boot_modules[line].file_pages); ENTER;
	}
	
	modules_addr = mem_alloc(&allocator, alloc_bytes, TRUE);
	if (modules_addr == NULL) {
		//PUTS(&scr, "Can't allocate modules_addr!");
		STAGE0_suicide(&scr, 1234);
	}
	DBG_PRINTF2("module raw data address: 0x%x total size: 0x%x", modules_addr, alloc_bytes); ENTER;

	// Now we can read the boot-modules to the memory:

	for (line = 0; line < num_of_lines; line++) {
		// read
		// TODO: check return value
		read_file(&hd_desc, boot_modules[line].file_name, modules_addr);

		// points to the data:
		boot_modules[line].file_data = modules_addr;

		// move to the next address
		modules_addr = (int8*)modules_addr + PAGE_SIZE * boot_modules[line].file_pages;
	}

	DBG_PUTS("Init the symbol table...");
	// init the symbol table:
	kgd->bootloader_symbols.symbols = (struct Symbol *)mem_alloc(&allocator, sizeof(struct Symbol)*MAX_PRIMITIVE_SYMBOLS + MAX_NAME_STORAGE,FALSE);
	kgd->bootloader_symbols.names_storage = ((char *)kgd->bootloader_symbols.symbols) + sizeof(struct Symbol)*MAX_PRIMITIVE_SYMBOLS;
	kgd->bootloader_symbols.index = 0;
	kgd->bootloader_symbols.names_storage_index = 0;
	
	
	kgd->APIC_base = mem_alloc_ex(&allocator, 0x400, FALSE, 0xfee00000);

	// load the modules:
	int ret2 = load_modules_and_run_kernel(kgd, boot_modules, &allocator, num_of_lines);
	if (ret2 != 0) STAGE0_suicide(&scr, ret2);

	// Should not reach here!
	STAGE0_suicide(&scr, 0xffffffff);
	
}

void STAGE0_suicide(ScreenHandle * screen_ptr, int error) {
	DBG_PRINTF2("Error %d (0x%x)", error, error);
	while (1) {};
	//int * s = (int*)0xffffffffffffffff;
	//*s = 0;
}





