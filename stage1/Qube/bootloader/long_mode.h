#pragma once

#define NULL 0

#define BOOT_TXT_FILE_MAX_SIZE 0x1000
#define MAX_BOOT_MODULES 0x10

#define PAGE_SIZE 0x1000
#define NUM_OF_PAGES(bytes) ((bytes + PAGE_SIZE-1)/PAGE_SIZE)
#define ALIGN_UP(addr) ((addr + PAGES_SIZE-1)/PAGE_SIZE*PAGE_SIZE)




#define MAX_LOADED_MODULES 0x100
typedef void * ModulesList[MAX_LOADED_MODULES];

struct KernelGlobalData {
	ModulesList * modules;
};

void STAGE0_suicide(int error);
void STAGE0_memset(void * addr, char c, int count);