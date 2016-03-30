#include "mem.h"
int32 init_allocator(Allocator *allocator){
	allocator->next_physical_nonvolatile = 0x101000;
	allocator->next_virtual_nonvolatile = 0xFF;//TODO
	allocator->next_physical_volatile = 0x407000;
	allocator->next_virtual_volatile = 0xffff800000008000;
	return -1;
}
char* mem_alloc(Allocator *allocator, int32 size, BOOL isVolatile){

}
char* virtual_commit(Allocator* allocator, int32 size){return 0;}
char* virtual_pages_alloc(Allocator* allocator, int32 num_of_pages, PAGE_ACCESS access){return 0;}