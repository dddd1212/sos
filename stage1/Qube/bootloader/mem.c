#include "mem.h"
#include "screen.h"
#define PTE(x) ((uint64*)(0xFFFFF68000000000 + (((((uint64)x) & 0x0000FFFFFFFFFFFF)>>12)<<3)))
#define PDE(x) PTE(PTE(x))
#define PPE(x) PTE(PDE(x))
#define PXE(x) PTE(PPE(x))

#define NONVOLATILE_VIRTUAL_START 0xfffff00000000000
#define VOLATILE_VIRTUAL_START 0x0000000000010000

#define VOLATILE_PHYSICAL_START 0x00000
#define VOLATILE_PHYSICAL_END 0x60000
#define NONVOLATILE_PHYSICAL_START 0x60000
#define NONVOLATILE_PHYSICAL_END 0x80000

#define NONVOLATILE_PHYSICAL_USED 1
#define FIRST_FREE_PHYSICAL (NONVOLATILE_PHYSICAL_START+NONVOLATILE_PHYSICAL_USED*0x1000)

#define PHYSICAL_PAGES_ENTRIES_PHY_ADDR 0xF000

#define PHYISICAL_PAGES_LIST 0xfffff00040000000

typedef struct {
	uint64 base;
	uint64 length;
	uint32 type;
	uint32 ex_attrs;
} PysicalRegionEntry;

int32 init_allocator(BootLoaderAllocator *allocator){
#ifdef DEBUG
	allocator->disable_non_volatile_allocs = FALSE;
#endif
	allocator->physical_pages_end = allocator->physical_pages_start = (uint64*)PHYISICAL_PAGES_LIST;

	*(PXE(NONVOLATILE_VIRTUAL_START)) = FIRST_FREE_PHYSICAL | 3;
	*(PPE(NONVOLATILE_VIRTUAL_START)) = (FIRST_FREE_PHYSICAL + 0x1000) | 3;
	// zero the new ptes page:
	for (int j = 0; j < (0x1000 / 8);j++) {
		*(PDE(NONVOLATILE_VIRTUAL_START)+j) = 0;
	}


	*(PPE(PHYISICAL_PAGES_LIST)) = (FIRST_FREE_PHYSICAL + 0x2000) | 3; // use same PXE
	*(PDE(PHYISICAL_PAGES_LIST)) = (FIRST_FREE_PHYSICAL + 0x3000) | 3; // use same PXE
	*(PTE(PHYISICAL_PAGES_LIST)) = (FIRST_FREE_PHYSICAL + 0x4000) | 3; // use same PXE


	// The list of physical regions from the BIOS are in the physical address PHYSICAL_PAGES_ENTRIES_PHY_ADDR.
	// This code sort the list.
	*(PTE(VOLATILE_VIRTUAL_START)) = (PHYSICAL_PAGES_ENTRIES_PHY_ADDR) | 3;
	PysicalRegionEntry* entries = (PysicalRegionEntry*)VOLATILE_VIRTUAL_START;
	uint32 i = 0;
	uint32 j = 0;
	while (entries[i].base != 0xFFFFFFFFFFFFFFFF) {
		j = i;
		while (j > 0 && entries[j].base < entries[j - 1].base) {
			PysicalRegionEntry e = entries[j];
			entries[j] = entries[j - 1];
			entries[j - 1] = e;
			j--;
		}
		i++;
	}
	

	// Now we build the list of physical pages.
	// First, we add the addresses 0x60000-0x80000
	for (uint64 addr = NONVOLATILE_PHYSICAL_START; addr < NONVOLATILE_PHYSICAL_END; addr+=0x1000) {
		*(allocator->physical_pages_end) = addr;
		allocator->physical_pages_end++;
	}

	// Consider the 6 pages we already used (5 here and 1 in real_mode.asm)
	allocator->next_physical_nonvolatile = allocator->physical_pages_start + 5 + NONVOLATILE_PHYSICAL_USED;

	uint64 last_addr = NONVOLATILE_PHYSICAL_END;
	for (i = 0; entries[i].base != 0xFFFFFFFFFFFFFFFF; i++) { 
		if (entries[i].type != 1) {
			continue;
		}
		uint64 start = (entries[i].base + 0xFFF) & 0xFFFFFFFFFFFFF000;
		uint64 end = (entries[i].base + entries[i].length) & 0xFFFFFFFFFFFFF000;
		if (end > last_addr) {
			if (start < last_addr) {
				start = last_addr;
			}
			for (uint64 cur = start; cur < end; cur += 0x1000, allocator->physical_pages_end++) {
				if ((((uint64)allocator->physical_pages_end) & 0x1fffff) == 0) {
					*PDE(allocator->physical_pages_end) = (*allocator->next_physical_nonvolatile)|3;
					allocator->next_physical_nonvolatile++;
				}
				if ((((uint64)allocator->physical_pages_end) & 0xfff) == 0) {
					*PTE(allocator->physical_pages_end) = (*allocator->next_physical_nonvolatile)|3;
					allocator->next_physical_nonvolatile++;
				}
				*allocator->physical_pages_end = cur;
			}
			last_addr = end;
		}
	}
	
	for (uint64 cur = VOLATILE_PHYSICAL_START; cur < VOLATILE_PHYSICAL_END; cur += 0x1000, allocator->physical_pages_end++) {
		if ((((uint64)allocator->physical_pages_end) & 0x1fffff) == 0) {
			*PDE(allocator->physical_pages_end) = (*allocator->next_physical_nonvolatile) | 3;
			allocator->next_physical_nonvolatile++;
		}
		if ((((uint64)allocator->physical_pages_end) & 0xfff) == 0) {
			*PTE(allocator->physical_pages_end) = (*allocator->next_physical_nonvolatile) | 3;
			allocator->next_physical_nonvolatile++;
		}
		*allocator->physical_pages_end = cur;
	}
	
	*(PTE(VOLATILE_VIRTUAL_START)) = 0;
	
	allocator->next_physical_volatile = allocator->physical_pages_end-0x800; // maximum of 0x800 (minus the 0x60 pages at 0-0x60000) pages. [~8M]

	allocator->next_virtual_nonvolatile = NONVOLATILE_VIRTUAL_START;
	allocator->next_virtual_volatile = VOLATILE_VIRTUAL_START;
	return 0;
}

void set_boot_info(BootLoaderAllocator *allocator, BootInfo* boot_info) {
	boot_info->physical_pages_start = allocator->physical_pages_start;
	boot_info->physical_pages_end = allocator->physical_pages_end;
	boot_info->physical_pages_current = allocator->next_physical_nonvolatile;
	boot_info->nonvolatile_virtual_start = (void*)NONVOLATILE_VIRTUAL_START;
	boot_info->nonvolatile_virtual_end = (void*)allocator->next_virtual_nonvolatile;
#ifdef DEBUG
	allocator->disable_non_volatile_allocs = TRUE;
#endif
}

void* mem_alloc_ex(BootLoaderAllocator *allocator, uint32 size, BOOL isVolatile, uint64 specific_phys_addr) {
#ifdef DEBUG
	if (isVolatile && allocator->disable_non_volatile_allocs) {
		return 0;
	}
#endif
	uint64 *next_physical;
	uint64 next_virtual;
	void *addr;
	if (isVolatile) {
		next_physical = allocator->next_physical_volatile;
		next_virtual = allocator->next_virtual_volatile;
	}
	else {
		next_physical = allocator->next_physical_nonvolatile;
		next_virtual = allocator->next_virtual_nonvolatile;
	}
	addr = (void*)next_virtual;

	uint32 num_of_pages = (size + 0xFFF) >> 12;
	for (int i = 0; i < num_of_pages; i++) {
		if (*PDE(next_virtual) == 0) { // need new PDE
			uint64* pte = PTE(next_virtual);
			// The physical addresses to store PTEs are always from the list.
			*PTE(pte) = (*next_physical)|3; // this is the pde
			next_physical++;
			// zero the new ptes page:
			uint64* pte_c = (uint64*)(((uint64)pte) & 0xFFFFFFFFFFFFF000);
			for (int j = 0; j < (0x1000 / 8);j++) {
				pte_c[j] = 0;
			}
		}
		if (specific_phys_addr == -1) { // Take physical address from the list
			*PTE(next_virtual) = (*next_physical) | 3;
		} else { // Set specific physical address:
			*PTE(next_virtual) = (specific_phys_addr + i * 0x1000) | 3;
		}
		next_virtual += 0x1000;
		next_physical++;
	}

	if (isVolatile) {
		allocator->next_physical_volatile = next_physical;
		allocator->next_virtual_volatile = next_virtual;
	}
	else {
		allocator->next_physical_nonvolatile = next_physical;
		allocator->next_virtual_nonvolatile = next_virtual;
	}
	return addr;
}
void* mem_alloc(BootLoaderAllocator *allocator, uint32 size, BOOL isVolatile) {
	return mem_alloc_ex(allocator, size, isVolatile, -1);
}

void* virtual_commit(BootLoaderAllocator* allocator, uint32 size, BOOL isVolatile){
	int32 num_of_pages = (size + 0xFFF) >> 12;
	void *addr;
	if (isVolatile) {
		addr = (void*)allocator->next_virtual_volatile;
		allocator->next_virtual_volatile += 0x1000 * num_of_pages;
	}
	else {
		addr = (void*)allocator->next_virtual_nonvolatile;
		allocator->next_virtual_nonvolatile += 0x1000 * num_of_pages;
	}
	return addr;
}

int32 alloc_committed(BootLoaderAllocator* allocator, uint32 size, void *addr){
	uint64 tempaddr;
	if ((uint64)addr % PAGE_SIZE != 0) return QFail;
	if (((uint64)addr) >= NONVOLATILE_VIRTUAL_START) {
		tempaddr = allocator->next_virtual_nonvolatile;
		allocator->next_virtual_nonvolatile = (uint64)addr;
		mem_alloc(allocator, size, FALSE);
		allocator->next_virtual_nonvolatile = tempaddr;
	}
	else {
		tempaddr = allocator->next_virtual_volatile;
		allocator->next_virtual_volatile = (uint64)addr;
		mem_alloc(allocator, size, TRUE);
		allocator->next_virtual_volatile = tempaddr;
	}
	return 0;
}

void * map_first_MB(BootLoaderAllocator *allocator) {
	// map 1MB non volotile memory from the start of the physical memory (0).
	return mem_alloc_ex(allocator, 1024 * 1024, FALSE, 0);
}

