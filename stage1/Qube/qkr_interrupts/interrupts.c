
#include "interrupts.h"
#include "../Common/Qube.h"
#include "../Common/intrinsics.h"
#include "../MemoryManager/memory_manager.h"
#include "../Common/spin_lock.h"
#include "lapic.h"
#include "processors.h"
#include "../qkr_acpi/acpi.h"
#include "ioapic.h"
#include "../libc/string.h"
#ifdef DEBUG
#include "../screen/screen.h"
#endif


IsScheduleNeededFunction g_is_schedule_call_needed = NULL;

InterruptDescriptor IDT[0x100];

ISR g_isrs[0x100];

BOOL init_interrupts() {
	memset(g_isrs, 0, sizeof(g_isrs));
	return (
		// Prepare the IDTs
	    init_IDTs()

	);
}

QResult qkr_main(KernelGlobalData * kgd) {
	// TODO: do it with the new memory module, and change the lapic_init interface to get on param.
 	lapic_init(kgd->APIC_base);
	//ACPITable * apic_table = get_acpi_table("APIA");
	//dump_table(apic_table);
	BOOL ret = (
		init_interrupts() &&
		// initialize the processors' specific data areas
		init_processors_data() &&

		// We run from the boot-strap processor,so we need to initialize its pcb:
		init_this_processor_control_block() &&

		// enable the local apic and the timer:
		lapic_start() &&

		ioapic_start()
		
	);

	
	if (ret) {
		// enable the interrupts in the current cpu
		enable_interrupts();
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
	for (int i = 0; i < 0x100; i++) {
		init_IDT(i, 0, DESC_TYPE_INTERRUPT, isrs_list[i]);
	}
	LIDT lidt;
	lidt.base = (uint64)(&(IDT[0]));
	lidt.limit = sizeof(IDT) - sizeof(IDT[0]); // limit 0 means one entry.
	__lidt((void*)&lidt);
	return TRUE;
}

QResult set_scheduler_interrupt_in_service() {
	this_processor_control_block()->scheduler_interrupt_in_service = TRUE;
	return QSuccess;
}

QResult issue_scheduler_interrupt() {
	if ((__get_cr8() >= (INT_SCHEDULER / 0x10)) || this_processor_control_block()->scheduler_interrupt_in_service) {
		this_processor_control_block()->scheduler_interrupt_pending = TRUE;
	}
	else {
		__int(INT_SCHEDULER);
	}
	return QSuccess;
}

BOOL is_scheduler_interrupt_in_service()
{
	return this_processor_control_block()->scheduler_interrupt_in_service;
}

QResult end_scheduler_interrupt() {
	disable_interrupts();
	this_processor_control_block()->scheduler_interrupt_in_service = FALSE;
	if (this_processor_control_block()->scheduler_interrupt_pending) {
		this_processor_control_block()->scheduler_interrupt_pending = FALSE;
		set_scheduler_interrupt_in_service();
		enable_interrupts();
		__int(INT_SCHEDULER);
	}
	__set_cr8(0);
	enable_interrupts();
	return QSuccess;
}
/*void register_is_shcedule_needed_function(IsScheduleNeededFunction f) {
	g_is_schedule_call_needed = f;
}*/

// This function handle all of the interrupts.
void handle_interrupts(ProcessorContext * regs) {

	// *** mask the less priority interrupts and enable the rest:
	// 1. Backup the TPR:
	regs->task_priority = __get_cr8();

start_handle_for_schedule_call: // We can replace it with {do while} but I think that this way is clearer.
#ifdef DEBUG
								//screen_write_string("Interrupt called!", TRUE);
	screen_set_color(regs->interrupt_vector % 8, (regs->interrupt_vector % 8)+1);
	screen_printf("Interrupt called: %d\n", regs->interrupt_vector, 0, 0, 0);
#endif
	// 2. set the TPR to be corellated to the vector:
	// TODO: TO THINK - because the masking is determines by the max(TPR, highest ISRV priority), so in regular interrupt we do not need to set this.
	//		 We need to do it just with interrupts that came not from the LAPIC: calls to the scheduler from the end of this function, and software interrupts(?).
	//		 Maybe we can to something better then set the cr8.
	
	__set_cr8(regs->interrupt_vector / 0x10);
	if (regs->interrupt_vector == INT_SCHEDULER) {
		this_processor_control_block()->scheduler_interrupt_in_service = TRUE;
	}

	enable_interrupts();
	// ***


	if (g_isrs[regs->interrupt_vector]) {
		g_isrs[regs->interrupt_vector](regs);
	} 
#ifdef DEBUG
	else {
		screen_set_color(regs->interrupt_vector % 8, (regs->interrupt_vector % 8) + 1);
		screen_printf("Interrupt has no interrupt vector! (%d)", regs->interrupt_vector, 0, 0, 0);
	}
#endif

	// disable interrupts, and call to schedule if needed
	disable_interrupts();
	
	// This also clear the cur vector bit from the ISRV so may change the CR8 register.
	g_lapic_regs->EOI = 0;

	if (regs->task_priority < (INT_SCHEDULER / 0x10) && this_processor_control_block()->scheduler_interrupt_pending) {// if we need to call to the scheduler
																							 //and we going to return to priority less then the scheduler
		this_processor_control_block()->scheduler_interrupt_pending = FALSE;
		// this_processor_control_block()->scheduler_interrupt_in_service = TRUE; //this is unnecessary because we set this flag after the goto.
		// call it:
		regs->interrupt_vector = INT_SCHEDULER;
		goto start_handle_for_schedule_call;

	}
	if (regs->interrupt_vector == INT_SCHEDULER) {
		this_processor_control_block()->scheduler_interrupt_in_service = FALSE;
	}
	// restore cr8:
	__set_cr8(regs->task_priority);

	//g_lapic_regs->EOI = 0;
#ifdef DEBUG
	//screen_write_string("Interrupt called!", TRUE);
	screen_set_color(regs->interrupt_vector % 8, (regs->interrupt_vector % 8) + 1);
	screen_printf("Interrupt returned: %d\n", regs->interrupt_vector, 0, 0, 0);
#endif
	return;
}


void enable_interrupts() {
	__sti();
}
void disable_interrupts() {
	__cli();
}

QResult register_isr(enum InterruptVectors isr_num, ISR isr) {
	if (g_isrs[isr_num] != NULL) return QFail;
	g_isrs[isr_num] = isr;
	return QSuccess;
}
