#include "memory_manager.h"

#define PTE(x) ((uint64*)(0xFFFFF68000000000 + (((((uint64)x) & 0x0000FFFFFFFFFFFF)>>12)<<3)))
#define PDE(x) PTE(PTE(x))
#define PPE(x) PTE(PDE(x))
#define PXE(x) PTE(PPE(x))

//addresses must start on PPE start area
#define MEMORY_MANAGEMENT_START_ADDRESS (0xffffc00000000000)
#define MODULES_START_ADDRESS (0xffffd00000000000)
#define KHEAP_START_ADDRESS (0xffffe00000000000)

#define MODULES_BITMAP_SIZE (0x100)
#define KHEAP_BITMAP_SIZE (0x400)

uint64 *g_physical_pages_current;
uint64 *g_physical_pages_end;
uint64 *g_physical_pages_start;
MemoryRegion g_regions[NUM_OF_REGION_TYPE];
X64_MEMORY_DESCRIPTOR g_memory_descriptors[3] = {
	{0},
	{0,0,0,10,1,0,1,0,0,1,0,0,0},
	{0,0,0,2 ,1,0,1,0,0,0,0,0,0}
};

X64_MEMORY_DESCRIPTOR_GDTR g_gdtr = { sizeof(g_memory_descriptors),g_memory_descriptors };

static uint64 pop_physical_page() {
	uint64 page = *g_physical_pages_current;
	g_physical_pages_current++;
	return page;
}

static uint64 push_physical_page(uint64 page) {
	g_physical_pages_current--;
	*g_physical_pages_current = page & 0xFFFFFFFFFF000;
}

BOOL get_bit(uint8* stream, uint32 bit_num) {
	return ((stream[bit_num / 8] & (1 << (bit_num % 8))) != 0);
}

void set_bit(uint8* stream, uint32 bit_num, BOOL value) {
	if (value) {
		stream[bit_num / 8] |= (1 << (bit_num % 8));
	}
	else {
		stream[bit_num / 8] &= ~(1 << (bit_num % 8));
	}
	
}

static void* get_region_start_address(KernelGlobalData* kgd, REGION_TYPE region_type) {
	switch (region_type) {
	case MEMORY_MANAGEMENT:
		return (void*) MEMORY_MANAGEMENT_START_ADDRESS;
	case MODULES:
		return kgd->boot_info->nonvolatile_virtual_start;
	case KHEAP:
		return (void*) KHEAP_START_ADDRESS;
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

static void set_bitmap(KernelGlobalData* kgd, REGION_TYPE region_type, MemoryRegion* region) {
	switch (region_type) {
	case MEMORY_MANAGEMENT:
		for (uint32 i = 0; i < region->bitmap_size; i++) {
			region->free_pages_bitmap[i] = 0xFF;
		}
		return;
	case MODULES:
		;
		uint32 i, j;
		uint32 num_of_pages = (((uint64)kgd->boot_info->nonvolatile_virtual_end) - ((uint64)kgd->boot_info->nonvolatile_virtual_start)) >> 12;
		for (i = 0; i < num_of_pages/8; i++) {
			region->free_pages_bitmap[i] = 0xFF;
		}
		region->free_pages_bitmap[i] = 0;
		for (j = 0; j < num_of_pages % 8; j++) {
			region->free_pages_bitmap[i] |= 1 << j;
		}
		for (i = i + 1; i < region->bitmap_size; i++) {
			region->free_pages_bitmap[i] = 0;
		}
		return;
	case KHEAP:
		for (uint32 i = 0; i < region->bitmap_size; i++) {
			region->free_pages_bitmap[i] = 0;
		}
		return;
	default:
		ASSERT(FALSE);
		return;
	}
}

static void add_physical_page(MemoryRegion* region, void* v_address, uint64 specific_physical_addr) {
	uint32 pde_offset = (((uint8*)v_address) - ((uint8*)region->start)) >> (12 + 9);
	uint32 ppe_offset = (((uint8*)v_address) - ((uint8*)region->start)) >> (12 + 9 + 9);
	if (region->PPE_use_count[ppe_offset] == 0) {
		*PPE(v_address) = pop_physical_page()|3;
	}
	if (region->PDE_use_count[pde_offset] == 0) {
		*PDE(v_address) = pop_physical_page()|3;
		region->PPE_use_count[ppe_offset]++;
	}
	if (specific_physical_addr == -1) {
		*PTE(v_address) = pop_physical_page()|3;
	}
	else {
		*PTE(v_address) = specific_physical_addr|(3| FLAG_OWNED_PAGE);
	}
	region->PDE_use_count[pde_offset]++;
}

static void remove_physical_page(MemoryRegion* region, void* v_address) {
	uint32 pde_offset = (((uint8*)v_address) - ((uint8*)region->start)) >> (12 + 9);
	uint32 ppe_offset = (((uint8*)v_address) - ((uint8*)region->start)) >> (12 + 9 + 9);
	if (!((*PTE(v_address)) & FLAG_OWNED_PAGE)) {
		push_physical_page(*PTE(v_address));
	}
	*PTE(v_address) = 0;
	region->PDE_use_count[pde_offset]--;
	if (region->PDE_use_count[pde_offset] == 0) {
		push_physical_page(*PDE(v_address));
		*PDE(v_address) = 0;
		region->PPE_use_count[ppe_offset]--;
	}
}

static void init_regions(KernelGlobalData* kgd) {
	uint8* memory_management_region_start_address = get_region_start_address(kgd,MEMORY_MANAGEMENT);


	for (uint32 i = MEMORY_MANAGEMENT; i < NUM_OF_REGION_TYPE; i++) {
		g_regions[i].free_pages_bitmap = memory_management_region_start_address + i*REGION_BITMAP_MAX_SIZE;
		g_regions[i].bitmap_size = get_region_bitmap_size(i);
		g_regions[i].start = get_region_start_address(kgd,i);
		// TODO: is this needed? or the loader already zeros this?
		for (uint32 j = 0; j < sizeof(g_regions[i].PPE_use_count) / sizeof(g_regions[i].PPE_use_count[0]); j++) {
			g_regions[i].PPE_use_count[j] = 0;
		}

		for (uint32 j = 0; j < sizeof(g_regions[i].PDE_use_count) / sizeof(g_regions[i].PDE_use_count[0]); j++) {
			g_regions[i].PDE_use_count[j] = 0;
		}

		// TODO: zero the pages
		for (uint64* pxe = PXE(g_regions[i].start); pxe <= PXE(((uint8*)(g_regions[i].start)) + (uint64)REGION_BITMAP_MAX_SIZE * 8 * 0x1000); pxe++) {
			if (*pxe == 0) {
				*pxe = pop_physical_page() | 3;
			}
		}

		for (uint32 j = 0; j < g_regions[i].bitmap_size; j+=PAGE_SIZE) {
			add_physical_page(&g_regions[MEMORY_MANAGEMENT], &g_regions[i].free_pages_bitmap[j],-1);
		}

		set_bitmap(kgd, i, &g_regions[i]);
	}
}

QResult qkr_main(KernelGlobalData* kgd) {
	__lgdt(&g_gdtr);
	g_physical_pages_current = kgd->boot_info->physical_pages_current;
	g_physical_pages_end = kgd->boot_info->physical_pages_end;
	g_physical_pages_start = kgd->boot_info->physical_pages_start;
	init_regions(kgd);
	init_kernel_heap();
	return QSuccess;
}

void* alloc_pages_(REGION_TYPE region, uint32 size) {
	uint8* addr = (uint8*)commit_pages_(region, size);
	for (uint8* cur = addr; cur < addr + size; cur += 0x1000) {
		add_physical_page(&g_regions[region], cur, -1);
	}
	return (void*)addr;
}

void* commit_pages_(REGION_TYPE region_type, uint32 size) {
	MemoryRegion* region = &g_regions[region_type];
	uint32 num_of_pages = ((size + 0xFFF) >> 12);
	uint32 cur_length = 0;
	uint32 cur_bit;
	for (cur_bit = 0;((cur_bit / 8) < region->bitmap_size) && (cur_length < num_of_pages+2); cur_bit++) {
		if (get_bit(region->free_pages_bitmap, cur_bit)) {
			cur_length = 0;
		}
		else {
			cur_length++;
		}
	}
	if (cur_length < num_of_pages+2) {
		// no pages found
		return NULL;
	}
	else {
		uint32 start_bit = cur_bit - 1 - num_of_pages;
		for (cur_bit = start_bit; cur_bit < start_bit + num_of_pages; cur_bit++) {
			set_bit(region->free_pages_bitmap, cur_bit, 1);
		}
		return (uint8*)region->start + 0x1000 * start_bit;
	}
}

void assign_committed_(void* addr, uint32 size, uint64 specific_physical) {
	// TODO: handle out of physical pages case.
	MemoryRegion* region = &g_regions[0];
	while (addr < region->start || addr > (void*)(((uint8*)region->start) + REGION_BITMAP_MAX_SIZE)) {
		region++;
	}
	for (uint8* cur = addr; cur < ((uint8*)addr) + size; cur += 0x1000) {
		add_physical_page(region, cur, specific_physical);
		if (specific_physical != -1) {
			specific_physical += 0x1000;
		}
	}
}

void unassign_committed_(void* addr, uint32 size) {
	MemoryRegion* region = &g_regions[0];
	while (addr < region->start || addr > (void*)(((uint8*)region->start) + REGION_BITMAP_MAX_SIZE)) {
		region++;
	}
	for (uint8* cur = addr; cur < ((uint8*)addr) + size; cur += 0x1000) {
		remove_physical_page(region, cur);
	}
}

void free_pages_(void* addr) {
	MemoryRegion* region = &g_regions[0];
	while (addr < region->start || addr >(void*)(((uint8*)region->start) + (uint64)0x8000*REGION_BITMAP_MAX_SIZE)) {
		region++;
	}
	uint32 start_bit;
	start_bit = ((uint64)addr - (uint64)region->start) / 0x1000;
	uint8* cur_page = (uint8*)addr;
	for (uint32 cur_bit = start_bit; get_bit(region->free_pages_bitmap, cur_bit); cur_bit++,cur_page+=0x1000) {
		remove_physical_page(region, (void*)cur_page);
		set_bit(region->free_pages_bitmap, cur_bit, 0);
	}
	return;
}




void* alloc_pages(REGION_TYPE region, uint32 size) {
	return alloc_pages_(region, size);
}

void* commit_pages(REGION_TYPE region_type, uint32 size) {
	return commit_pages_(region_type, size);
}

void assign_committed(void* addr, uint32 size, uint64 specific_physical) {
	assign_committed_(addr, size, specific_physical);
}

void unassign_committed(void* addr, uint32 size) {
	unassign_committed_(addr, size);
}

void free_pages(void* addr) {
	free_pages_(addr);
}