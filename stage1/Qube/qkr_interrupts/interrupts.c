#include "interrupts.h"
#include "../Common/Qube.h"
#include "../Common/intrinsics.h"
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

#define APIC_REG_BASE 0xfee00000

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
	init_interrupts();
	__int(0x23);
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
	if (__rdmsr(IA32_APIC_BASE_MSR) & 0xfffff100 != APIC_REG_BASE) return FALSE;

	// 
	// Check if IOAPIC exist:
	// TODO: For now, we won't check it.
	//       We have 2 options: 1. To assumes that it exist in the default address.
	//							2. Using the ACPI to determine if it exist and where.
	//		 For now, we will assume the default address.
	return TRUE;
}

BOOL disable_old_PIC() {
	__out8(0xa1, 0xff);
	__out8(0x21, 0xff);
}

BOOL init_IDT(uint8 vector, uint8 dpl, DescType type, uint64 handler_addr) {
	IDT[vector].dpl = dpl; // 0 or 3
	IDT[vector].ist = 0; // We not use this mechanism.
	IDT[vector].offset1 = handler_addr & 0xffff;
	IDT[vector].offset2 = (handler_addr >> 16) & 0xffff;
	IDT[vector].offset3 = (handler_addr >> 32);
	IDT[vector].present = 1; // The handlers always present in the memory.
	IDT[vector].reserved = 0;
	IDT[vector].selector = KERNEL_DATA_SEGMENT; // Interrupts always runs in kernel mode.
	IDT[vector].type = type; // interrupt or trap
	IDT[vector].zero = 0;
	IDT[vector].zeros = 0;
	return TRUE;
}

BOOL init_IDTs() {
	// Init all of the vectors to be interrupts:
	for (int i = 0; i < 0x100; i++) {
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
	screen_write_string("Interrupt called!", TRUE);
#endif
	return;
}


BOOL init_APIC() {
	return TRUE;
}

BOOL enable_APIC() {
	return TRUE;
}

BOOL enable_interrupts() {
	__sti();
}