#include "mem.h"
#define PTE(x) ((int64*)(0xFFFFF6C000000000 + ((((int64)x - 0xFFFF800000000000)>>12)<<3)))
int32 init_allocator(BootLoaderAllocator *allocator){
	allocator->next_physical_nonvolatile = 0x101000;
	allocator->next_virtual_nonvolatile = 0xfffff00000000000;//TODO
	allocator->next_physical_volatile = 0x407000;
	allocator->next_virtual_volatile = 0xffff800000008000;
	return -1;
}

void* mem_alloc(BootLoaderAllocator *allocator, int32 size, BOOL isVolatile){
	int64 next_physical, next_virtual;
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

	int32 num_of_pages = (size + 0xFFF) >> 12;
	for (int i = 0; i < num_of_pages; i++) {
		if ((next_virtual & 0x1fffff) == 0) { // need new PDE
			int64* pte = PTE(next_virtual);
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
char* virtual_commit(BootLoaderAllocator* allocator, int32 size){return 0;}
char* virtual_pages_alloc(BootLoaderAllocator* allocator, int32 num_of_pages, PAGE_ACCESS access){return 0;}