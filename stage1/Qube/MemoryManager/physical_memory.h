#ifndef __physical_memory__h__
#define __physical_memory__h__

#define PHYSICAL_MAX_SIZE 0x100000
typedef struct {
	void * virtual_address;
	uint64 physical_start;
	uint64 physical_end;
	BOOL is_allocated;
} PhysicalMemory;

QResult _physical_alloc(PhysicalMemory * pmem, uint64 phys_addr, uint64 size);

// This API gives an access to physical memory.
// To use it, you have to create PhysicalMemory struct and call init_physical_memory_struct.
// When finish using it, you have to call fini_physical_memory_struct.
// To get virtual pointer, call to physical_memory_get_ptr. check that the return value is not NULL.
// 
// The implemntation is such way:
// When you call NOT in the first time to physical_memory_get_ptr, its faster to do it if 
// your new pointer is in the range of the old pointer + PHYSICAL_MAX_SIZE.
//
// You cannot get pointer larger then PHYSICAL_MAX_SIZE.
//
// PhysicalMemory is not thread safe!
EXPORT void physical_memory_init(PhysicalMemory * pmem);
EXPORT void physical_memory_fini(PhysicalMemory * pmem);
EXPORT void * physical_memory_get_ptr(PhysicalMemory * pmem, uint64 phys_addr, uint64 size);

#endif // __physical_memory__h__
