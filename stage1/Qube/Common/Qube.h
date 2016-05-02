#ifndef __QUBE_H__
#define __QUBE_H__
// C general defines:
typedef char			 int8;
typedef unsigned char	 uint8;
typedef short			 int16;
typedef unsigned short	 uint16;
typedef int				 int32;
typedef unsigned int	 uint32;
typedef long long		 int64;
typedef unsigned long long uint64;
typedef uint64			 size_t;
#define NULL 0
#define FALSE 0
#define TRUE 1
typedef int32 BOOL;


// TODO: Do with it something
#define ASSERT

typedef int32 QResult;
#define QSuccess 0
#define QFail -1

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

// Little ugly. maybe we need to define the ScreenHandle in another place??
typedef struct {
	short * start_screen_ptr;
	short * cur_screen_ptr;
	short * end_screen_ptr;
} ScreenHandle;

typedef struct {
	uint64 *physical_pages_start;
	uint64 *physical_pages_end;
	uint64 *physical_pages_current;
	ScreenHandle * scr;
} BootInfo;

typedef struct {
	struct PrimitiveSymbols bootloader_symbols; // symbols for the primitive loader.
	ModulesList * modules;
	BootInfo *boot_info;
	void * first_MB; // pointer to the first MB of pysical memory.
} KernelGlobalData;

typedef struct {
	KernelGlobalData * kgd;
} PEB;

// DLLMain function header: (Every module may implements this function):
// TODO: Change kgd to PEB
QResult qkr_main(KernelGlobalData * kgd);

#include "intrinsics.h"



#endif // __QUBE_H__