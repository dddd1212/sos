#pragma once

// Reserve for us numOfPages pages.
void * STAGE0_virtual_commit(unsigned int numOfPages);

// allocate commited pages.
int STAGE0_virtual_pages_alloc(void * requestedAddr, unsigned int numOfPages, enum PAGE_ACCESS access);