#include "../Common/Qube.h"
#include "physical_memory.h"
#include "memory_manager.h"
#include "../libc/string.h"
QResult _physical_alloc(PhysicalMemory * pmem, uint64 phys_addr, uint64 size) {
	if (pmem->is_allocated) {
		unassign_committed(pmem->virtual_address, pmem->physical_end - pmem->physical_start);
	}
	else {
		pmem->virtual_address = commit_pages(KHEAP, PHYSICAL_MAX_SIZE);
		if (pmem->virtual_address == NULL) return QFail;
	}
	assign_committed(pmem->virtual_address, size, phys_addr);
	pmem->is_allocated = TRUE;
	pmem->physical_start = phys_addr;
	pmem->physical_end = phys_addr + size;
	return QSuccess;
}

void physical_memory_init(PhysicalMemory * pmem) {
	memset(pmem, 0, sizeof(PhysicalMemory));
}

void physical_memory_fini(PhysicalMemory * pmem) {
	if (pmem->is_allocated) {
		free_pages(pmem->virtual_address);
	}
	return;
}

void * physical_memory_get_ptr(PhysicalMemory * pmem, uint64 phys_addr, uint64 size) {
	
	uint64 diff = phys_addr - phys_addr / PAGE_SIZE * PAGE_SIZE; // align to page.
	phys_addr -= diff;
	size += diff;
	QResult ret;
	if (size > PHYSICAL_MAX_SIZE) return NULL; // Can't ensure that we have such size mapped.
	if (pmem->is_allocated) {
		if (phys_addr >= pmem->physical_start && phys_addr + size < pmem->physical_end) { // The pointer is already mapped to our virtual space.
			return (void *)((uint64)pmem->virtual_address + phys_addr - pmem->physical_start + diff);
		}
		if (phys_addr >= pmem->physical_start && pmem->physical_start + PHYSICAL_MAX_SIZE > phys_addr + size) {// The pointer is not in our virtual space, but its after it, and close enough to be less then PHYSICAL_MAX_SIZE.
																											   // realloc, but in the same physical_start.
			ret = _physical_alloc(pmem, pmem->physical_start, size + phys_addr - pmem->physical_start);
			if (ret == QFail) return NULL;
		}
		// In all other cases, just realloc:
		ret = _physical_alloc(pmem, phys_addr, size);
		if (ret == QFail) return NULL;
	}
	else {
		ret = _physical_alloc(pmem, phys_addr, size);
		if (ret == QFail) return NULL;
	}
	return pmem->virtual_address + diff;
}