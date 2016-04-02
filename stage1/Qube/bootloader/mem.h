#ifndef MEM_H
#define MEM_H

#include "Qube.h"
typedef struct {
	int64 next_physical_nonvolatile;
	int64 next_virtual_nonvolatile;
	int64 next_physical_volatile;
	int64 next_virtual_volatile;
} Allocator;
typedef enum {
	PAGE_ACCESS_NONE = 0x0,
	PAGE_ACCESS_READ = 0x1,
	PAGE_ACCESS_WRITE = 0x2,
	PAGE_ACCESS_RW = 0x3,
	PAGE_ACCESS_EXEC = 0x4,
	PAGE_ACCESS_RX = 0x5,
	PAGE_ACCESS_WX = 0x6,
	PAGE_ACCESS_RWX = 0x7,
}PAGE_ACCESS;
int32 init_allocator(Allocator *allocator);
void* mem_alloc(Allocator *allocator, int32 size, BOOL isVolatile);
char* virtual_commit(Allocator* allocator, int32 size);
char* virtual_pages_alloc(Allocator* allocator, int32 num_of_pages, PAGE_ACCESS access);
#endif