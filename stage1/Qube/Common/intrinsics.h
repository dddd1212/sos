//TODO: verify the assumption of the volatile registers

#ifndef INTRINSICS_H
#define INTRINSICS_H

#include "Qube.h"

typedef int volatile SpinLock;
static inline int8 __in8(uint16 port) __attribute__((always_inline));
static inline void __out8(uint16 port, uint8 data) __attribute__((always_inline));
static inline void __insw(uint16 port, uint32 count, void *addr) __attribute__((always_inline));
static inline BOOL __qube_sync_bool_compare_and_swap(SpinLock * p, int old_val, int new_val) __attribute__((always_inline));
static inline void __qube_mm_pause() __attribute__((always_inline));
static inline void __qube_memory_barrier() __attribute__((always_inline));
static inline void __cpuid(int code, uint32 * eax_out, uint32 * edx_out)  __attribute__((always_inline));
static inline void __sti() __attribute__((always_inline));
static inline void __cli() __attribute__((always_inline));
static inline uint64 __rdmsr(uint32 msr_id) __attribute__((always_inline));
static inline void __wrmsr(uint32 msr_id, uint64 msr_value) __attribute__((always_inline));
static inline void __invlpg(void* addr) __attribute__((always_inline));
static inline void __lidt(void* addr) __attribute__((always_inline));
static inline void __sidt(void* addr) __attribute__((always_inline));
static inline void io_wait() __attribute__((always_inline));

static inline uint64 __get_cr8() __attribute__((always_inline));
static inline void __set_cr8(uint64 cr8) __attribute__((always_inline));
static inline void* __get_processor_block() __attribute__((always_inline));
// #define __int(N) // implements later in the file.

#define ___HELP_READ_FROM_GS_STRINGIFY(a) #a
#define __READ_FROM_GS(ret, offset) { asm("mov %%gs:" ___HELP_READ_FROM_GS_STRINGIFY(offset) ",%0" : "=A" (ret)); }
#define __SET_GS(value)
							
static inline void __qube_memory_barrier() {
	asm volatile (""); // acts as a memory barrier.
}
static inline void __qube_mm_pause() {
	//_mm_pause();
	__asm__("pause;");
}

static inline BOOL __qube_sync_bool_compare_and_swap(SpinLock * p, int old_val, int new_val) {
	return __sync_bool_compare_and_swap(p, old_val, new_val);
}

// TODO: add Clobber List
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
	__asm__(
		".intel_syntax noprefix;"
		"out dx, al;"
		".att_syntax;"
		: 
		: "a"(data),"d"(port)
	);
	return;
}

static inline void __insw(uint16 port, uint32 count, void *addr){
	// TODO: verify that [mov ecx,%1] zeros the upper 32bit of rcx
	__asm__(
		".intel_syntax noprefix;"
		"rep insw;"
		".att_syntax;"
		:
		: "d"(port),"c"(count),"D"(addr)
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
static inline void __lidt(void* addr) {
	__asm__(
		".intel_syntax noprefix;"
		"lidt [%0];"
		".att_syntax;"
		:
	: "r"(addr)
		);
	return;
}
static inline void __sidt(void * addr) {
	__asm__(
		".intel_syntax noprefix;"
		"sidt [%0];"
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

static inline void __wrmsr(uint32 msr_id, uint64 msr_value)
{
	uint32 edx = msr_value >> 32;
	uint32 eax = msr_value & 0xffffffff;
	
	asm volatile ("wrmsr" : : "c" (msr_id), "a" (eax), "d" (edx));
}

static inline uint64 __rdmsr(uint32 msr_id)
{
	uint64 msr_value;
	asm volatile ("rdmsr" : "=A" (msr_value) : "c" (msr_id));
	return msr_value;
}

static inline void __invlpg(void* addr) {
	__asm__(
		".intel_syntax noprefix;"
		"invlpg [%0];"
		".att_syntax;"
		:
	: "r"(addr)
		);
	return;
}

static inline void __sti() {
	__asm__("sti");
	return;
}
static inline void __cli() {
	__asm__("cli");
	return;
}

static inline void io_wait() {
	// According to OSDev this is done like this in linux. We assumes that nobody uses this port.
	asm volatile ("outb %%al, $0x80" : : "a"(0));
}

static inline uint64 __get_cr8() {
	uint64 ret;
	__asm__("movq %%cr8, %q[var]" : [var] "=q" (ret));
	return ret;
}
static inline void __set_cr8(uint64 cr8) {
	__asm__("movq %q[var], %%cr8" : [var] "=q" (cr8));
}

static inline void* __get_processor_block() {
	void* pb;
	__asm__(
		".intel_syntax noprefix;"
		"mov %0,gs:[0];"
		".att_syntax;"
		: "=r"(pb)
		: 
		);
	return pb;
}

#define __int(n) __asm__("int %0" : : "N"((n)) : "cc", "memory");
#endif