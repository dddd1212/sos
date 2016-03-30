#include "mem.h"
// Reserve for us numOfPages pages.
void * STAGE0_virtual_commit(unsigned int numOfPages) {
	return 0;
}

// allocate commited pages.
int STAGE0_virtual_pages_alloc(void * requestedAddr, unsigned int numOfPages, enum PAGE_ACCESS access) {
	return 0;
}