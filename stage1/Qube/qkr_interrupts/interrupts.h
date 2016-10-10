#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__
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

	uint64 task_priority; // the function handle_interrupts fill this field (the task priority).
	uint64 specific_isr_return_address; // the return address to the isr_wrapperXXX.
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



enum InterruptVectors {
	// Lowest priority:
	PIC1_IRQ0 = 0x20, // PIC timer
	PIC1_IRQ1 = 0x21,
	PIC1_IRQ2 = 0x22,
	PIC1_IRQ3 = 0x23,
	PIC1_IRQ4 = 0x24,
	PIC1_IRQ5 = 0x25,
	PIC1_IRQ6 = 0x26,
	PIC1_IRQ7 = 0x27,
	PIC2_IRQ0 = 0x28,
	PIC2_IRQ1 = 0x29,
	PIC2_IRQ2 = 0x2a,
	PIC2_IRQ3 = 0x2b,
	PIC2_IRQ4 = 0x2c,
	PIC2_IRQ5 = 0x2d,
	PIC2_IRQ6 = 0x2e,
	PIC2_IRQ7 = 0x2f,
	APIC_SPURIOUS = 0x30,

	
	INT_SCHEDULER = 0x40,
	
	APIC_USER_DEFINED_START = 0x41,
	APIC_USER_DEFINED_END = 0xa0,



	APIC_KEYBOARD_CONTROLLER = 0xa3,


	

	APIC_TIMER = 0xf2,
	
};

BOOL init_interrupts();
BOOL init_IDTs();

typedef BOOL(*IsScheduleNeededFunction)();
//EXPORT void register_is_shcedule_needed_function(IsScheduleNeededFunction f);
EXPORT QResult set_scheduler_interrupt_in_service();
EXPORT QResult end_scheduler_interrupt();
EXPORT QResult issue_scheduler_interrupt();
//static void serve_scheduling() { __int(INT_SCHEDULER); };

EXPORT void enable_interrupts();
EXPORT void disable_interrupts();
BOOL init_IDT(uint8 vector, uint8 dpl, DescType type, uint64 handler_addr);

typedef void(*ISR)(ProcessorContext * regs);



EXPORT QResult register_isr(enum InterruptVectors isr_num, ISR isr);

#endif // __INTERRUPTS_H__