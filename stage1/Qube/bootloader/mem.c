#include "mem.h"
// CR: GIlad - You should define just one number (0 to 256) that will say the choise that we choose for the PTE address.
//			   Its very difficult to understand what you try to do here and y.
#define PTE(x) ((int64*)(0xFFFFF6C000000000 + ((((int64)x - 0xFFFF800000000000)>>12)<<3)))
#define NONVOLATILE_VIRTUAL_START 0xfffff00000000000
#define VOLATILE_VIRTUAL_START 0xffff800000008000
int32 init_allocator(BootLoaderAllocator *allocator){
	// CR: Gilad - again, remove these numbers..
	*((uint64*)0xFFFFF6FB7DBEDF00) = 0x101000 | 3;
	*((uint64*)0xFFFFF6FB7DBE0000) = 0x102000 | 3;
	*((uint64*)0xFFFFF6FB7C000000) = 0x103000 | 3;
	allocator->next_physical_nonvolatile = 0x104000;
	allocator->next_virtual_nonvolatile = NONVOLATILE_VIRTUAL_START;
	allocator->next_physical_volatile = 0x407000;
	allocator->next_virtual_volatile = VOLATILE_VIRTUAL_START;
	return -1; 
}

void* mem_alloc(BootLoaderAllocator *allocator, uint32 size, BOOL isVolatile){
	uint64 next_physical, next_virtual;
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
		// CR: gilad - #define SIZE_OF_????? 0x1fffff
		if ((next_virtual & 0x1fffff) == 0) { // need new PDE
			uint64* pte = PTE(next_virtual);
			// CR: gilad - #define ?@#??!@# 3
			*PTE(pte) = next_physical|3; // this is the pde
			next_physical += 0x1000;
			// zero the new ptes page:
			for (int j = 0; j < (0x1000 / 8);j++) {
				pte[j] = 0;
			}
		}
		*PTE(next_virtual) = next_physical | 3;
		next_virtual += 0x1000;
		next_physical += 0x1000;
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


void* virtual_commit(BootLoaderAllocator* allocator, uint32 size, BOOL isVolatile){
	// CR: GIlad - change to SIZE_OF_PAGE - 1
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

