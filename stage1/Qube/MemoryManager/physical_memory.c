#include "../Common/Qube.h"
#include "physical_memory.h"
#include "memory_manager.h"
#include "../libc/string.h"
// phys_addr must be page aligned.
// size must be page aligned.
QResult _physical_alloc(PhysicalMemory * pmem, uint64 phys_addr, uint64 size) {
	if (pmem->_is_allocated) {
		unassign_committed(pmem->_virtual_start, pmem->_size);
		physical_memory_fini(pmem);
		if (physical_memory_init(pmem) == QFail) return QFail;
	}
	assign_committed(pmem->_virtual_start, size, phys_addr);
	pmem->_is_allocated = TRUE;
	if (pmem->_virtual_start == NULL) return QFail;
	pmem->_physical_start = phys_addr;
	pmem->_size = size;
	return QSuccess;
}

QResult physical_memory_init(PhysicalMemory * pmem) {
	memset(pmem, 0, sizeof(PhysicalMemory));
	pmem->_virtual_start = commit_pages(KHEAP, PHYSICAL_MAX_SIZE);
	if (pmem->_virtual_start == NULL) return QFail;
	return QSuccess;
}

void physical_memory_fini(PhysicalMemory * pmem) {
	free_pages(pmem->_virtual_start);
	return;
}

void * physical_memory_get_ptr(PhysicalMemory * pmem, uint64 phys_addr, uint64 size) {
	pmem->addr = NULL;
	uint64 diff = phys_addr - ALIGN_DOWN(phys_addr); // align to page.
	phys_addr -= diff; // align to page.
	size += diff;
	size = ALIGN_UP(size);
	QResult ret;
	if (size > PHYSICAL_MAX_SIZE) return NULL; // Can't ensure that we have such size mapped.
	if (pmem->_is_allocated) {
		if (phys_addr >= pmem->_physical_start && phys_addr + size < pmem->_physical_start + pmem->_size) { // The pointer is already mapped to our virtual space.
			return (void *)((uint64)pmem->_virtual_start + phys_addr - pmem->_physical_start + diff);
		}
		/*
		if (phys_addr >= pmem->_physical_start && pmem->_physical_start + PHYSICAL_MAX_SIZE > phys_addr + size) {// The pointer is not in our virtual space, but its after it, and close enough to be less then PHYSICAL_MAX_SIZE.
																											     // realloc, but in the same physical_start.
			ret = _physical_alloc(pmem, pmem->physical_start, size + phys_addr - pmem->physical_start);
			if (ret == QFail) return NULL;
		}
		*/
		// In all other cases, just realloc:
		ret = _physical_alloc(pmem, phys_addr, size);
		if (ret == QFail) return NULL;
	}
	else {
		ret = _physical_alloc(pmem, phys_addr, size);
		if (ret == QFail) return NULL;
	}

	pmem->addr = pmem->_virtual_start + diff;
	return pmem->addr;
}