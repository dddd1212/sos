#ifndef MEMORY_MANAGER
#define MEMORY_MANAGER
#include "../Common/Qube.h"
#define REGION_MAXIMUM_SIZE 0x100000
#define REGION_BITMAP_MAX_SIZE (REGION_MAXIMUM_SIZE-sizeof(MemoryRegion))
typedef enum {
	MEMORY_MANAGEMENT,
	MODULES,
	KHEAP,
	NUM_OF_REGION_TYPE
}REGION_TYPE;
typedef struct {
	void* start;
	uint32 bitmap_size;
	uint8 free_pages_bitmap[0];
} MemoryRegion;
void init_memory_manager(KernelGlobalData* kgd);
void* alloc_pages(REGION_TYPE region, uint32 size);
void* commit_pages(REGION_TYPE region, uint32 size);
void assign_committed(void* addr, uint32 size);
void unassign_committed(void* addr, uint32 size);

#endif