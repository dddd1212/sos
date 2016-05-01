#ifndef __QUBE_H__
#define __QUBE_H__
// C general defines:
#define int8 char
#define uint8 unsigned char
#define int16 short
#define uint16 unsigned short
#define int32 int
#define uint32 unsigned int
#define int64 long long
#define uint64 unsigned long long

#define NULL 0
#define FALSE 0
#define TRUE 1
#define BOOL int32

#define ASSERT

typedef int32 QResult;

// base configuration and basic defines:
#define BOOT_TXT_FILE_MAX_SIZE 0x1000
#define MAX_BOOT_MODULES 0x10

#define PAGE_SIZE 0x1000
#define NUM_OF_PAGES(bytes) ((bytes + PAGE_SIZE-1)/PAGE_SIZE)
#define ALIGN_UP(addr) ((addr + PAGES_SIZE-1)/PAGE_SIZE*PAGE_SIZE)

#define MAX_LOADED_MODULES 0x100
///// We need to split this file to couple of files...


// Global kernel data:
typedef void * ModulesList[MAX_LOADED_MODULES];

struct Symbol {
	char * name;
	uint64 addr;
};

#define MAX_PRIMITIVE_SYMBOLS 0x100
#define MAX_NAME_STORAGE 0x100*0x10
struct PrimitiveSymbols {
	int index;
	struct Symbol * symbols;
	int names_storage_index;
	char * names_storage;
};
typedef struct {
	uint64 *physical_pages_start;
	uint64 *physical_pages_end;
	uint64 *physical_pages_current;
} BootInfo;
typedef struct KernelGlobalData {
	struct PrimitiveSymbols bootloader_symbols; // symbols for the primitive loader.
	ModulesList * modules;
	BootInfo *boot_info;
	void * first_MB; // pointer to the first MB of pysical memory.
} KernelGlobalData;

// DLLMain function header: (Every module may implements this function):
QResult qmo_main();

#include "intrinsics.h"



#endif // __QUBE_H__