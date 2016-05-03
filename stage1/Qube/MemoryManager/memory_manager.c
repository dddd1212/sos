#include "memory_manager.h"

#define PTE(x) ((uint64*)(0xFFFFF68000000000 + (((((uint64)x) & 0x0000FFFFFFFFFFFF)>>12)<<3)))
#define PDE(x) PTE(PTE(x))
#define PPE(x) PTE(PDE(x))
#define PXE(x) PTE(PPE(x))

//addresses must start on PPE start area
#define MEMORY_MANAGEMENT_START_ADDRESS 0xffffc00000000000
#define MODULES_START_ADDRESS 0xffffd00000000000
#define KHEAP_START_ADDRESS 0xffffe00000000000

#define MODULES_BITMAP_SIZE 0x100
#define KHEAP_BITMAP_SIZE 0x100

uint64 *g_physical_pages_current;
uint64 *g_physical_pages_end;
uint64 *g_physical_pages_start;
MemoryRegion g_regions[NUM_OF_REGION_TYPE];

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
		return MODULES_START_ADDRESS;
	case KHEAP:
		return KHEAP_START_ADDRESS;
	default:
		ASSERT(FALSE);
		return NULL;
	}
}

static uint32 get_region_bitmap_size(REGION_TYPE region_type) {
	switch (region_type) {
	case MEMORY_MANAGEMENT:
		return (NUM_OF_REGION_TYPE*REGION_BITMAP_MAX_SIZE) >> 15;
	case MODULES:
		return MODULES_BITMAP_SIZE;
	case KHEAP:
		return KHEAP_BITMAP_SIZE;
	default:
		ASSERT(FALSE);
		return 0;
	}
}


static void add_physical_page(MemoryRegion* region, void* v_address) {
	uint32 pde_offset = (((uint8*)region->start) - ((uint8*)v_address)) >> (12 + 9);
	uint32 ppe_offset = (((uint8*)region->start) - ((uint8*)v_address)) >> (12 + 9 + 9);
	if (region->PPE_use_count[ppe_offset] == 0) {
		*PPE(v_address) = pop_physical_page();
	}
	if (region->PPE_use_count[pde_offset] == 0) {
		*PDE(v_address) = pop_physical_page();
	}
	*PTE(v_address) = pop_physical_page();
	region->PPE_use_count[ppe_offset]++;
	region->PPE_use_count[pde_offset]++;
}

static void init_regions() {
	uint8* memory_management_region_start_address = get_region_start_address(MEMORY_MANAGEMENT);


	for (uint32 i = MEMORY_MANAGEMENT; i < NUM_OF_REGION_TYPE; i++) {
		g_regions[i].free_pages_bitmap = memory_management_region_start_address + REGION_BITMAP_MAX_SIZE;
		g_regions[i].bitmap_size = get_region_bitmap_size(i);
		g_regions[i].start = get_region_start_address(i);
		// TODO: is this needed? or the loader already zeros this?
		for (uint32 j = 0; j < sizeof(g_regions[i].PPE_use_count); j++) {
			g_regions[i].PPE_use_count[j] = 0;
		}

		for (uint32 j = 0; j < sizeof(g_regions[i].PDE_use_count); j++) {
			g_regions[i].PDE_use_count[j] = 0;
		}

		// TODO: zero the pages
		for (uint64* pxe = PXE(g_regions[i].start); pxe <= PXE(g_regions[i].start + REGION_BITMAP_MAX_SIZE * 8 * 0x1000); pxe++) {
			if (*pxe == 0) {
				*pxe = pop_physical_page();
			}
		}

		if (i == MEMORY_MANAGEMENT) {
			add_physical_page(&g_regions[MEMORY_MANAGEMENT], memory_management_region_start_address);
			ASSERT(g_regions[MEMORY_MANAGEMENT].bitmap_size <= 0x1000);
			for (uint32 i = 0; i < g_regions[MEMORY_MANAGEMENT].bitmap_size; i++) {
				g_regions[MEMORY_MANAGEMENT].free_pages_bitmap[i] = 0xFF;
			}
		}
		else {
			for (uint32 i = 0; i < g_regions[i].bitmap_size; i++) {
				g_regions[MEMORY_MANAGEMENT].free_pages_bitmap[i] = 0x00;
			}
		}
	}
}

void init_memory_manager(KernelGlobalData* kgd) {
	g_physical_pages_current = kgd->boot_info->physical_pages_current;
	g_physical_pages_end = kgd->boot_info->physical_pages_end;
	g_physical_pages_start = kgd->boot_info->physical_pages_start;
	init_regions();
}

