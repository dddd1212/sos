#include "../Common/Qube.h"

//EXPORT QResult qkr_main(KernelGlobalData * kgd);
typedef struct {
	uint64 DS;
	uint64 ES;
	uint64 FS;
	uint64 GS;
	uint64 RAX;
	uint64 RBX;
	uint64 RCX;
	uint64 RDX;
	uint64 RBP;
	uint64 RDI;
	uint64 RSI;
	uint64 R8;
	uint64 R9;
	uint64 R10;
	uint64 R11;
	uint64 R12;
	uint64 R13;
	uint64 R14;
	uint64 R15;

	// must be the last ones
	uint64 interrupt_vector; // This is pushed by software in isrs.S in isr_wrapperXXX functions.
	uint64 error_code; // This may be push by the software or by the hardware, depends on the exception.
	// pushed by the processor.
	uint64 RIP;
	uint64 CS;
	uint64 RFLAGS;
	uint64 RSP;
	uint64 SS;
} ProcessorContext;

typedef struct {
	uint16 offset1;
	uint16 selector;
	union {
		uint16 attr;
		struct {
			uint16 ist : 3; // Interrupt stack table
			uint16 zeros : 5;
			uint16 type : 4;
			uint16 zero : 1;
			uint16 dpl : 2;
			uint16 present: 1;  // segment present flag
		};
	};
	uint16 offset2;
	uint32 offset3;
	uint32 reserved;
} InterruptDescriptor;

void handle_interrupts(ProcessorContext * regs);
// declare the IDT. implements in interrupts.c
extern InterruptDescriptor IDT[0x100];
typedef struct __attribute__((packed)){
	uint16 limit;
	uint64 base;
} LIDT;
typedef enum {
	DESC_TYPE_INTERRUPT = 14,
	DESC_TYPE_TRAP = 15,
} DescType;

// implements in isrs.S
extern uint64 isrs_list[0x100] asm("isrs_list");

// TODO: If we will use for system-call the opcode sysenter, so we won't need it.
//	     So change it to 0x101 or remove it and fix the code that using it.
#define SYSTEM_CALL_VECTOR 0x51 // 'Q'