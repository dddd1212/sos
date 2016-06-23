
#include "interrupts.h"
#include "../Common/Qube.h"
#include "../Common/intrinsics.h"
#include "../MemoryManager/memory_manager.h"
#include "../Common/spin_lock.h"
#include "processors.h"
#ifdef DEBUG
#include "../screen/screen.h"
#endif

InterruptDescriptor IDT[0x100];

BOOL init_interrupts() {
	return (
		// Prepare the IDTs
	    init_IDTs() &&

		// enable the interrupts in the current cpu
		enable_interrupts()

	);
}

QResult qkr_main(KernelGlobalData * kgd) {
	// TODO: do it with the new memory module, and change the lapic_init interface to get on param.
	lapic_init(kgd->APIC_base);
	BOOL ret = (
		// initialize the processors' specific data areas
		init_processors_data() &&

		// We run from the boot-strap processor,so we need to initialize its pcb:
		init_this_processor_control_block() &&

		// enable the local apic:
		lapic_start() &&

		// TODO: Init the IOAPIC, enable the IOAPIC. We probably need the ACPI first to use it properely
		// 
		// Check if IOAPIC exist:
		// TODO: For now, we won't check it.
		//       We have 2 options: 1. To assumes that it exist in the default address.
		//							2. Using the ACPI to determine if it exist and where.
		//		 For now, we will assume the default address.

		init_interrupts()
	);

	


	// apic_timer_init
	while (1) {

	}
	if (ret) {
		return QSuccess;
	}
	return QFail;
	
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
	switch (regs->interrupt_vector) {
	
	}
	return;
}


BOOL enable_interrupts() {
	__sti();
	return TRUE;
}


