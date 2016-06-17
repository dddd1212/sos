#ifndef HEAP_H
#define HEAP_H
#include "../Common/Qube.h"

#define HEAP_INIT_FUNC init_kernel_heap
#define HEAP_FREE_FUNC kheap_free
#define HEAP_ALLOC_FUNC kheap_alloc

QResult HEAP_INIT_FUNC();
EXPORT void* HEAP_ALLOC_FUNC(uint32 size);
EXPORT void HEAP_FREE_FUNC(void* addr);


#endif