//TODO: verify the assumption of the volatile registers

#ifndef INTRINSICS_H
#define INTRINSICS_H

#include "Qube.h"
typedef int volatile * SpinLock;
static inline int8 __in8(uint16 port) __attribute__((always_inline));
static inline void __out8(uint16 port, uint8 data) __attribute__((always_inline));
static inline void __insw(uint16 port, uint32 count, void *addr) __attribute__((always_inline));
static inline BOOL __qube_sync_bool_compare_and_swap(SpinLock p, int old_val, int new_val) __attribute__((always_inline));
static inline void __qube_mm_pause() __attribute__((always_inline));
static inline void __qube_memory_barrier() __attribute__((always_inline));
static inline void __cpuid(int code, uint32 * eax_out, uint32 * edx_out);






static inline void __qube_memory_barrier() {
	asm volatile (""); // acts as a memory barrier.
}
static inline void __qube_mm_pause() {
	_mm_pause();
}

static inline BOOL __qube_sync_bool_compare_and_swap(SpinLock p, int old_val, int new_val) {
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

static inline void __lgdt(void* addr) {
	__asm__(
		".intel_syntax noprefix;"
		"lgdt [%0];"
		".att_syntax;"
		:
		: "r"(addr)
	);
	return;
}

static inline void __cpuid(int code, uint32 * eax_out, uint32 * edx_out)
{
	asm volatile ("cpuid" : "=a"(*eax_out), "=d"(*edx_out) : "0"(code) : "rbx", "rcx");
}

inline void __wrmsr(uint32 msr_id, uint64 msr_value)
{
	asm volatile ("wrmsr" : : "c" (msr_id), "A" (msr_value));
}

inline uint64 __rdmsr(uint32 msr_id)
{
	uint64 msr_value;
	asm volatile ("rdmsr" : "=A" (msr_value) : "c" (msr_id));
	return msr_value;
}
#endif