#include "interrupts.h"
#include "../Common/Qube.h"
#include "../Common/intrinsics.h"
#include "../MemoryManager/memory_manager.h"

#ifdef DEBUG
#include "../screen/screen.h"
#endif


BOOL disable_old_PIC();
BOOL init_IDTs();
BOOL init_APIC();
BOOL enable_APIC();

BOOL init_APIC();
BOOL is_APIC_exist();
BOOL enable_interrupts();
BOOL init_IDT(uint8 vector, uint8 dpl, DescType type, uint64 handler_addr);
//void disable_APIC();

// TODO: change to define, after Dror will implement the malloc function in the memory module.
#define APIC_REG_BASE 0xfee00000

#define APIC_DWORD(x) uint32 x; \
					  char x##pad[12];
typedef struct {
	APIC_DWORD(reserved0); // 0x0
	APIC_DWORD(reserved10); // 0x10
	APIC_DWORD(local_apic_id); // 0x20
	APIC_DWORD(local_apic_version); // 0x30
	APIC_DWORD(reserved40); // 0x40
	APIC_DWORD(reserved50); // 0x50
	APIC_DWORD(reserved60); // 0x60
	APIC_DWORD(reserved70); // 0x70
	APIC_DWORD(TPR); // 0x80 - Task Priority Register
	APIC_DWORD(APR); // 0x90 - Arbitration Priority Register
	APIC_DWORD(PPR); // 0xa0 - Processor Priority Register
	APIC_DWORD(EOI); // 0xb0 - End Of Interrupt Register
	APIC_DWORD(RRD); // 0xc0 - Remote Read Register
	APIC_DWORD(local_destination); // 0xd0
	APIC_DWORD(Destination_format); // 0xe0
	APIC_DWORD(Spurious_interrupt_vector); // 0xf0
	APIC_DWORD(ISR0); // 0x100
	APIC_DWORD(ISR1); // 0x110
	APIC_DWORD(ISR2); // 0x120
	APIC_DWORD(ISR3); // 0x130
	APIC_DWORD(ISR4); // 0x140
	APIC_DWORD(ISR5); // 0x150
	APIC_DWORD(ISR6); // 0x160
	APIC_DWORD(ISR7); // 0x170
	APIC_DWORD(TMR0); // 0x180
	APIC_DWORD(TMR1); // 0x190
	APIC_DWORD(TMR2); // 0x1a0
	APIC_DWORD(TMR3); // 0x1b0
	APIC_DWORD(TMR4); // 0x1c0
	APIC_DWORD(TMR5); // 0x1d0
	APIC_DWORD(TMR6); // 0x1e0
	APIC_DWORD(TMR7); // 0x1f0
	APIC_DWORD(IRR0); // 0x200
	APIC_DWORD(IRR1); // 0x210
	APIC_DWORD(IRR2); // 0x220
	APIC_DWORD(IRR3); // 0x230
	APIC_DWORD(IRR4); // 0x240
	APIC_DWORD(IRR5); // 0x250
	APIC_DWORD(IRR6); // 0x260
	APIC_DWORD(IRR7); // 0x270
	APIC_DWORD(error_status); // 0x280
	APIC_DWORD(reserved290); // 0x290
	APIC_DWORD(reserved2a0); // 0x2a0
	APIC_DWORD(reserved2b0); // 0x2b0
	APIC_DWORD(reserved2c0); // 0x2c0
	APIC_DWORD(reserved2d0); // 0x2d0
	APIC_DWORD(reserved2e0); // 0x2e0
	APIC_DWORD(LVT_CMCI); // 0x2f0
	APIC_DWORD(ICR_low); // 0x300
	APIC_DWORD(ICR_high); // 0x310
	APIC_DWORD(LVT_timer); // 0x320
	APIC_DWORD(LVT_thermal); // 0x330
	APIC_DWORD(LVT_performance); // 0x340
	APIC_DWORD(LVT_LINT0); // 0x350
	APIC_DWORD(LVT_LINT1); // 0x360
	APIC_DWORD(LVT_error); // 0x370
	APIC_DWORD(initial_count); // 0x380
	APIC_DWORD(current_count); // 0x390
	APIC_DWORD(reserved3a0); // 0x3a0
	APIC_DWORD(reserved3b0); // 0x3b0
	APIC_DWORD(reserved3c0); // 0x3c0
	APIC_DWORD(reserved3d0); // 0x3d0
	APIC_DWORD(divide_conf); // 0x3e0
	APIC_DWORD(reserved3f0); // 0x3f0
} APICRegisters;

static APICRegisters * g_apic_regs = 0;
InterruptDescriptor IDT[0x100];

BOOL init_interrupts() {
	
	return (
		// First thing we want to validate that we support the hardware
		is_APIC_exist() &&

		// Disable the legacy PIC because we won't use it.
		disable_old_PIC() &&

		// Prepare the IDTs
	    init_IDTs() &&

		// Configure the APIC
		init_APIC() &&

		// Now we ready to enable the APIC
		enable_APIC() &&

		// Init the IOAPIC, enable the IOAPIC. We probably need the ACPI first to use it properely

		// enable the interrupts in the current cpu
		enable_interrupts()

	);
}

QResult qkr_main(KernelGlobalData * kgd) {
	screen_write_string("LIDT INITED!", TRUE);
	//screen_printf("My temp string %s %d", "1111", 1234, 0, 0);
	g_apic_regs = kgd->APIC_base;
	//g_apic_regs = 2;
	init_interrupts();

	while (1) {

	}
	//__int(0x23);
	int i;
	i += 1;
	return i;
}

BOOL is_APIC_exist() {
	// Check if APIC exist:
	int code = 1;
	int eax_out;
	int edx_out;
#define IA32_APIC_BASE_MSR 0x1b
	__cpuid(code, &eax_out, &edx_out);
	if (!(edx_out & (1 << 9))) return FALSE;
	// Validate that the base address ofr the APIC registers are as expected:
	
	// TODO: the field is more then 32bit. we need to see what the size and & with these bits.
	if (__rdmsr(IA32_APIC_BASE_MSR) & 0xfffff100 != APIC_REG_BASE) return FALSE;

	// 
	// Check if IOAPIC exist:
	// TODO: For now, we won't check it.
	//       We have 2 options: 1. To assumes that it exist in the default address.
	//							2. Using the ACPI to determine if it exist and where.
	//		 For now, we will assume the default address.
	return TRUE;
}
/* reinitialize the PIC controllers, giving them specified vector offsets
rather than 8h and 70h, as configured by default */
#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)
#define ICW1_ICW4	0x01		/* ICW4 (not) needed */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */

#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */

/*
arguments:
offset1 - vector offset for master PIC
vectors on the master become offset1..offset1+7
offset2 - same for slave PIC: offset2..offset2+7
*/
void io_wait() {
	asm volatile ("outb %%al, $0x80" : : "a"(0));
}
void PIC_remap(int offset1, int offset2)
{
	unsigned char a1, a2;

	a1 = __in8(PIC1_DATA);                        // save masks
	a2 = __in8(PIC2_DATA);

	__out8(PIC1_COMMAND, ICW1_INIT + ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
	io_wait();
	__out8(PIC2_COMMAND, ICW1_INIT + ICW1_ICW4);
	io_wait();
	__out8(PIC1_DATA, offset1);                 // ICW2: Master PIC vector offset
	io_wait();
	__out8(PIC2_DATA, offset2);                 // ICW2: Slave PIC vector offset
	io_wait();
	__out8(PIC1_DATA, 4);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	io_wait();
	__out8(PIC2_DATA, 2);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
	io_wait();

	__out8(PIC1_DATA, ICW4_8086);
	io_wait();
	__out8(PIC2_DATA, ICW4_8086);
	io_wait();

	__out8(PIC1_DATA, a1);   // restore saved masks.
	__out8(PIC2_DATA, a2);
}
BOOL disable_old_PIC() {
	PIC_remap(0x80, 0x90);
	//__out8(0xa1, 0xff); // disable the pic
	//__out8(0x21, 0xff); // disable the pic
	// TODO: We have to understand whats going on here. according to bochs code, the only way to clear the interrupt that pending now is throut reset the PIC.
	//	     The problem is that this is enable the pic, so we have atomic problem...
	//__out8(0x20, 0x11); // reset the pic
	/*
	uint16 port;
	uint8 value;
	for (int IRQline = 0; IRQline < 16; IRQline++) {
		if (IRQline < 8) {
			port = 0x21;
		}
		else {
			port = 0xa1;
			IRQline -= 8;
		}
		value = __in8(port) | (1 << IRQline);
		__out8(port, value);
	}
	__out8(0xa1, 0xff); // disable the pic
	__out8(0x21, 0xff); // disable the pic
	// Clear any already pending interrupts:
	// TODO: Check if this is needed in non-bochs environment
	for (int i = 0; i < 20; i++) {
		__out8(0xA0, 0x20);
		__out8(0x20, 0x20);
	}
	*/
		
	uint64 msr = __rdmsr(IA32_APIC_BASE_MSR);
	//__wrmsr(IA32_APIC_BASE_MSR, msr & (~(1 << 11)));
}

BOOL init_IDT(uint8 vector, uint8 dpl, DescType type, uint64 handler_addr) {
	IDT[vector].dpl = dpl; // 0 or 3
	IDT[vector].ist = 0; // We not use this mechanism.
	IDT[vector].offset1 = handler_addr & 0xffff;
	IDT[vector].offset2 = (handler_addr >> 16) & 0xffff;
	IDT[vector].offset3 = (handler_addr >> 32);
	IDT[vector].present = 1; // The handlers always present in the memory.
	IDT[vector].reserved = 0;
	IDT[vector].selector = KERNEL_CODE_SEGMENT; // Interrupts always runs in kernel mode.
	IDT[vector].type = type; // interrupt or trap
	IDT[vector].zero = 0;
	IDT[vector].zeros = 0;
	return TRUE;
}

BOOL init_IDTs() {
	// Init all of the vectors to be interrupts:
	//screen_printf("isrs_list = 0x%x, 0x%x", &isrs_list, isrs_list,0,0);
	for (int i = 0; i < 0x100; i++) {
		//screen_printf("isrs_list[%d] = 0x%x", i, isrs_list[i],0,0);
		
		if (i == SYSTEM_CALL_VECTOR) {
			init_IDT(i, 3, DESC_TYPE_INTERRUPT, isrs_list[i]); // User allow to use system call, so the dpl is 3.
		} else {
			init_IDT(i, 0, DESC_TYPE_INTERRUPT, isrs_list[i]);
		}
	}
	LIDT lidt;
	lidt.base = (uint64)(&(IDT[0]));
	lidt.limit = sizeof(IDT);
	__lidt((void*)&lidt);
	return TRUE;
}


// This function handle all of the interrupts.
void handle_interrupts(ProcessorContext * regs) {
#ifdef DEBUG
	//screen_write_string("Interrupt called!", TRUE);
	screen_printf("Interrupt called: %d\n", regs->interrupt_vector,0,0,0);
#endif
	return;
}


BOOL init_APIC() {
	APICRegisters * apic = g_apic_regs;
	uint32 spur = apic->Spurious_interrupt_vector;
	uint64 msr = __rdmsr(IA32_APIC_BASE_MSR);
	
	return TRUE;
}

BOOL enable_APIC() {
	return TRUE;
}

BOOL enable_interrupts() {
	__sti();
}