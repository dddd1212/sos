//TODO: verify the assumption of the volatile registers

#ifndef INTRINSICS_H
#define INTRINSICS_H

#include "Qube.h"
inline int8 __in8(uint16 port) __attribute__((always_inline));
inline void __out8(uint16 port, uint8 data) __attribute__((always_inline));
inline void __insw(uint16 port, uint32 count, void *addr) __attribute__((always_inline));

inline int8 __in8(uint16 port){
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

inline void __out8(uint16 port, uint8 data){
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

inline void __insw(uint16 port, uint32 count, void *addr){
	// TODO: verify that [mov ecx,%1] zeros the upper 32bit of rcx
	__asm__(
		".intel_syntax noprefix;"
		"mov dx, %0;"
		"mov ecx,%1;"
		"push rdi;"
		"mov rdi,%2;"
		"rep insw;"
		"pop rdi;"
		".att_syntax;"
		:
		: "r"(port),"r"(count),"r"(addr)
	);
}


#endif