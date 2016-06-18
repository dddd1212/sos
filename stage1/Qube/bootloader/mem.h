#ifndef MEM_H
#define MEM_H



#define BPB_ADDR 0x0000000000007C00

#include "../Common/Qube.h"
typedef struct {
	uint64* next_physical_nonvolatile;
	uint64 next_virtual_nonvolatile;
	uint64* next_physical_volatile;
	uint64 next_virtual_volatile;

	uint64* physical_pages_start;
	uint64* physical_pages_end;
#ifdef DEBUG
	BOOL disable_non_volatile_allocs;
#endif
} BootLoaderAllocator;

typedef enum {
	PAGE_ACCESS_NONE = 0x0,
	PAGE_ACCESS_READ = 0x1,
	PAGE_ACCESS_WRITE = 0x2,
	PAGE_ACCESS_RW = 0x3,
	PAGE_ACCESS_EXEC = 0x4,
	PAGE_ACCESS_RX = 0x5,
	PAGE_ACCESS_WX = 0x6,
	PAGE_ACCESS_RWX = 0x7,
}PAGE_ACCESS;

int32 init_allocator(BootLoaderAllocator *allocator);
void set_boot_info(BootLoaderAllocator *allocator, BootInfo* boot_info);
void* mem_alloc(BootLoaderAllocator *allocator, uint32 size, BOOL isVolatile);

// commit and allocate 'size' bytes with the allocator 'allocator' using the 'specific_phys_addr' physical address.
void* mem_alloc_ex(BootLoaderAllocator *allocator, uint32 size, BOOL isVolatile, uint64 specific_phys_addr);
void* virtual_commit(BootLoaderAllocator* allocator, uint32 size, BOOL isVolatile);
int32 alloc_committed(BootLoaderAllocator* allocator, uint32 size, void *addr);
void * map_first_MB(BootLoaderAllocator *allocator);
#endif
