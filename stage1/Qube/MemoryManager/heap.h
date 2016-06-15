#ifndef HEAP_H
#define HEAP_H
#include "../Common/Qube.h"
EXPORT void* kheap_alloc(uint32 size);
EXPORT void kheap_free(void* addr);


#endif