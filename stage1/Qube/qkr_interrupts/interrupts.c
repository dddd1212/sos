#include "interrupts.h"
//#include "../Common/Qube.h"
#include "../Common/intrinsics.h"
BOOL disable_old_PIC();
BOOL init_IDTs();
BOOL init_APIC();
BOOL enable_APIC();

BOOL init_APIC();
BOOL is_APIC_exist();

void disable_APIC();

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

BOOL init_IDTs() {
	return 0;
}


// This function handle all of the interrupts.
void handle_interrupts(ProcessorContext * regs) {
	
}