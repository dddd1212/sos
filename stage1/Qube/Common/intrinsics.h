//TODO: verify the assumption of the volatile registers

#ifndef INTRINSICS_H
#define INTRINSICS_H

#include "Qube.h"
static inline int8 __in8(uint16 port) __attribute__((always_inline));
static inline void __out8(uint16 port, uint8 data) __attribute__((always_inline));
static inline void __insw(uint16 port, uint32 count, void *addr) __attribute__((always_inline));
static inline bool __qube_sync_bool_compare_and_swap(void *p, int old_val, int new_val) __attribute__((always_inline));
static inline void __qube_mm_pause() __attribute__((always_inline));
static inline void __qube_memory_barrier() __attribute__((always_inline));


typedef int volatile * SpinLock;

void inline spin_init(SpinLock p) {
	*p = 0;
}

void inline spin_lock(SpinLock p)
{
	while (!__qube_sync_bool_compare_and_swap(p, 0, 1))
	{
		while (*p) __qube_mm_pause();
	}
}

void inline spin_unlock(SpinLock p)
{
	__qube_memory_barrier();
	*p = 0;
}

static inline void __qube_memory_barrier() {
	asm volatile (""); // acts as a memory barrier.
}
static inline void __qube_mm_pause() {
	_mm_pause();
}

static inline bool __qube_sync_bool_compare_and_swap(SpinLock p, int old_val, int new_val) {
	return __sync_bool_compare_and_swap(p, old_val, new_val);
}

static inline int8 __in8(uint16 port){
	int8 res;
	__asm__(
		".intel_syntax noprefix;"
		"mov dx, %1;"
		"in al, dx;"
		"mov %0, al;"
		".att_syntax;"
		: "=r"(res)
		: "r"(port)
	);
	return res;
}

static inline void __out8(uint16 port, uint8 data){
	int8 res;
	__asm__(
		".intel_syntax noprefix;"
		"mov dx, %1;"
		"mov al, %0;"
		"out dx, al;"
		".att_syntax;"
		: 
		: "r"(data),"r"(port)
	);
	return;
}

static inline void __insw(uint16 port, uint32 count, void *addr){
	// TODO: verify that [mov ecx,%1] zeros the upper 32bit of rcx
	__asm__(
		".intel_syntax noprefix;"
		"mov rdi,%2;"
		"mov cx, %0;"
		"mov edx,%1;"
		"xchg ecx, edx;"
		"push rdi;"
		"rep insw;"
		"pop rdi;"
		".att_syntax;"
		:
		: "r"(port),"r"(count),"r"(addr)
	);
}


#endif