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

struct KernelGlobalData {
	ModulesList * modules;
};



#endif // __QUBE_H__