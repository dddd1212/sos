#include "Qube.h"
typedef struct {
	int64 next_physical_nonvolatile;
	int64 next_virtual_nonvolatile;
	int64 next_physical_volatile;
	int64 next_virtual_volatile;
} Allocator;

int32 init_allocator(Allocator *allocator);
char* mem_alloc(Allocator *allocator, int32 size, BOOL isVolatile);
