typedef struct {
	int32 tbd;
} Allocator;

int32 init_allocator(Allocator *allocator){return -1;}
char* mem_alloc(Allocator *allocator, int32 size, BOOL isVolatile){return 0;}
