#pragma once

#define NULL 0

#define BOOT_TXT_FILE_MAX_SIZE 0x1000
#define MAX_BOOT_MODULES 0x10

#define PAGE_SIZE 0x1000
#define NUM_OF_PAGES(bytes) ((bytes + PAGE_SIZE-1)/PAGE_SIZE)
#define ALIGN_UP(addr) ((addr + PAGES_SIZE-1)/PAGE_SIZE*PAGE_SIZE)


enum PAGE_ACCESS {
	PAGE_ACCESS_NONE = 0x0,
	PAGE_ACCESS_READ = 0x1,
	PAGE_ACCESS_WRITE = 0x2,
	PAGE_ACCESS_RW = 0x3,
	PAGE_ACCESS_EXEC = 0x4,
	PAGE_ACCESS_RX = 0x5,
	PAGE_ACCESS_WX = 0x6,
	PAGE_ACCESS_RWX = 0x7,
};

#define MAX_LOADED_MODULES 0x100
typedef void * ModulesList[MAX_LOADED_MODULES];

struct KernelGlobalData {
	ModulesList * modules;
};

void STAGE0_suicide(int error);
void STAGE0_memset(void * addr, char c, int count);