#ifndef MEMORY_MANAGER
#define MEMORY_MANAGER
#include "../Common/Qube.h"
#define REGION_BITMAP_MAX_SIZE 0x100000

typedef struct {
	uint16 limit_low;
	uint16 base_low;
	uint8 base_middle;
	uint8 type : 4;
	uint8 S : 1; // System (if cleared)
	uint8 DLP : 2;
	uint8 P : 1; // segment present
	uint8 limit_high : 4;
	uint8 AVL : 1; 
	uint8 L : 1;
	uint8 D_B : 1;
	uint8 G : 1; // granularity
	uint8 base_high;

} __attribute__((packed)) X64_MEMORY_DESCRIPTOR;

typedef struct {
	uint16 Limit;
	X64_MEMORY_DESCRIPTOR* Base;
} __attribute__((packed)) X64_MEMORY_DESCRIPTOR_GDTR;

typedef enum {
	MEMORY_MANAGEMENT,
	MODULES,
	KHEAP,
	NUM_OF_REGION_TYPE
}REGION_TYPE;

typedef struct {
	void* start;
	uint32 bitmap_size;
	uint8* free_pages_bitmap;
	uint16 PDE_use_count[(8*REGION_BITMAP_MAX_SIZE) / (0x200)];
	uint16 PPE_use_count[(8*REGION_BITMAP_MAX_SIZE) / (0x200 * 0x200)];
} MemoryRegion;

EXPORT QResult qkr_main(KernelGlobalData* kgd);
EXPORT void* alloc_pages(REGION_TYPE region, uint32 size);
EXPORT void* commit_pages(REGION_TYPE region, uint32 size);
EXPORT void assign_committed(void* addr, uint32 size, uint64 specific_physical);
EXPORT void unassign_committed(void* addr, uint32 size);
EXPORT void free_pages(void* addr);
#endif