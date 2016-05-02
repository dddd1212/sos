#include "memory_manager.h"

#define PTE(x) ((uint64*)(0xFFFFF68000000000 + (((((uint64)x) & 0x0000FFFFFFFFFFFF)>>12)<<3)))
#define PDE(x) PTE(PTE(x))
#define PPE(x) PTE(PDE(x))
#define PXE(x) PTE(PPE(x))

#define MEMORY_MANAGEMENT_START_ADDRESS 0xffffc00000000000
#define MODULES_MANAGEMENT_START_ADDRESS 0xffffd00000000000
#define KHEAP_MANAGEMENT_START_ADDRESS 0xffffe00000000000
uint64 *g_physical_pages_current;
uint64 *g_physical_pages_end;
uint64 *g_physical_pages_start;
MemoryRegion* g_regions[NUM_OF_REGION_TYPE];

static uint64 pop_physical_page() {
	uint64 page = *g_physical_pages_current;
	g_physical_pages_current++;
	return page;
}

static uint64 push_physical_page(uint64 page) {
	g_physical_pages_current--;
	*g_physical_pages_current = page & 0xFFFFFF000;
}

static void* get_region_start_address(REGION_TYPE region_type) {
	switch (region_type) {
	case MEMORY_MANAGEMENT:
		return MEMORY_MANAGEMENT_START_ADDRESS;
	case MODULES:
		return MODULES_MANAGEMENT_START_ADDRESS;
	case KHEAP:
		return KHEAP_MANAGEMENT_START_ADDRESS;
	default:
		ASSERT(FALSE);
	}
}

static void init_regions() {
	uint8* memory_management_region_start_address = get_region_start_address(MEMORY_MANAGEMENT);
	*PXE(memory_management_region_start_address) = pop_physical_page(); // TODO: zero the pages
	*PPE(memory_management_region_start_address) = pop_physical_page();
	*PDE(memory_management_region_start_address) = pop_physical_page();
	*PTE(memory_management_region_start_address) = pop_physical_page();

	MemoryRegion* memory_management_region = (MemoryRegion*)memory_management_region_start_address;
	g_regions[MEMORY_MANAGEMENT] = memory_management_region;

	ASSERT(sizeof(MemoryRegion) + memory_management_region->bitmap_size <= 0x1000);

	memory_management_region->start = memory_management_region;
	memory_management_region->bitmap_size = (NUM_OF_REGION_TYPE*REGION_MAXIMUM_SIZE) >> 15;
	for (uint32 i = 0; i < memory_management_region->bitmap_size; i++) {
		memory_management_region->free_pages_bitmap[i] = 0xFF;
	}

	for (uint32 i = MEMORY_MANAGEMENT+1; i < NUM_OF_REGION_TYPE; i++) {
		MemoryRegion* region = (MemoryRegion*)(memory_management_region_start_address + REGION_MAXIMUM_SIZE*i);
		region->bitmap_size = 0;
		region->start = get_region_start_address(i);
		g_regions[i] = region;
	}
}

void init_memory_manager(KernelGlobalData* kgd) {
	g_physical_pages_current = kgd->boot_info->physical_pages_current;
	g_physical_pages_end = kgd->boot_info->physical_pages_end;
	g_physical_pages_start = kgd->boot_info->physical_pages_start;
	init_regions();
}

