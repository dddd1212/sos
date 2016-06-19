#include "interrupts.h"
#include "../Common/Qube.h"
#include "../Common/intrinsics.h"
#include "../MemoryManager/memory_manager.h"
#include "../Common/spin_lock.h"
#ifdef DEBUG
#include "../screen/screen.h"
#endif


BOOL configure_old_PIC();
BOOL init_IDTs();

BOOL init_APIC(char apic_address_for_ipi);
BOOL enable_APIC();

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

typedef struct {
	APICRegisters * regs;
	uint64 bus_freq;
	
	BOOL is_timer_run;
	SpinLock is_timer_run_lock;

} APICContext;
static APICContext g_apic_context;
//static APICRegisters * g_apic_regs = 0;
InterruptDescriptor IDT[0x100];
//static uint64 g_apic_bus_freq = 0; // number of ticks per second. Not initialized yet.
BOOL init_interrupts() {
	
	return (
		// First thing we want to validate that we support the hardware
		is_APIC_exist() &&

		// Configure the legacy PIC to be disabled except from the timer.
		configure_old_PIC() &&

		// Prepare the IDTs
	    init_IDTs() &&

		// Configure the APIC
		init_APIC(0) &&

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
	g_apic_context.regs = kgd->APIC_base;
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
	// Validate that the base address of the APIC registers are as expected:
	
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

#define PIC1_COMMAND	0x20
#define PIC1_DATA		0x21
#define PIC2_COMMAND	0xa0
#define PIC2_DATA		0xa1

#define PIT_FREQ 1193182

typedef enum {
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
	APIC_TIMER = 0x30,
	APIC_SPURIOUS = 0x39,
} InterruptVectors;

BOOL configure_old_PIC() {
	// Configure the timer (PIT):
	__out8(0x43, 0x36); // We going to configure channel 0.
	__out8(0x40, 0x1); // Low byte
	__out8(0x40, 0x0); // High byte
	
	// re-init the PIC:
	__out8(PIC1_COMMAND, 0x11);
	io_wait();
	__out8(PIC2_COMMAND, 0x11);
	io_wait();
	__out8(PIC1_DATA, PIC1_IRQ0);                 // ICW2: Master PIC vector offset
	io_wait();
	__out8(PIC2_DATA, PIC2_IRQ0);                 // ICW2: Slave PIC vector offset
	io_wait();
	__out8(PIC1_DATA, 4);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	io_wait();
	__out8(PIC2_DATA, 2);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
	io_wait();
	__out8(PIC1_DATA, 0x01);
	io_wait();
	__out8(PIC2_DATA, 0x01);
	io_wait();
	__out8(PIC1_DATA, 0xfe);   // mask everything except timer.
	io_wait();
	__out8(PIC2_DATA, 0xff);   // mask everything.
	
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


// apic_timer api:
BOOL apic_timer_init(); // Should be called only once before any calls to apic_timer_start, apic_timer_stop and apic_timer_get_time_ellapsed.
BOOL apic_timer_start(uint64 timer_us);
BOOL apic_timer_stop();
uint64 apic_timer_get_time_ellapsed();

BOOL apic_timer_init() {
	g_apic_context.is_timer_run = FALSE;
	spin_init(&g_apic_context.is_timer_run_lock);
	if (g_apic_context.bus_freq == 0) return FALSE;
}

BOOL apic_timer_start(uint64 timer_us) {
	APICRegisters * apic = g_apic_context.regs;

	
	// Calculate how many ticks we need:
	uint64 ticks = g_apic_context.bus_freq * timer_us / 1000 / 1000;
	uint32 divide_number = 1;
	char divide_value = 0b11111111; // we will use only the 4 low bits.
	
	while (ticks > 0xffffffff) {
		divide_number *= 2;
		divide_value += 1; // we will use only three low bits.
		if (divide_value = 0b100) {
			divide_value = 0b1000; // The 3rd bit must be zero...
		}
		ticks /= 2;
		if (divide_number > 128) { // We can't divide more then 128..
			return FALSE; // time too long to wait it in the timer.
		}
	}
	apic->divide_conf = divide_value & 0b1011;
	apic->initial_count = (uint32)ticks;
	
	spin_lock(&g_apic_context.is_timer_run_lock);
	g_apic_context.is_timer_run = TRUE;
	apic->LVT_timer = 0; // unmask the interrupt. one-shot mode.
	spin_unlock(&g_apic_context.is_timer_run_lock);
	return TRUE;

}

BOOL apic_timer_stop() {
	APICRegisters * apic = g_apic_context.regs;
	
	BOOL ret;
	spin_lock(&g_apic_context.is_timer_run_lock);
	apic->LVT_timer = 0x10000; // mask the interrupt.
	ret = g_apic_context.is_timer_run;
	g_apic_context.is_timer_run = FALSE;
	spin_unlock(&g_apic_context.is_timer_run_lock);
	return ret; // TRUE - if we stopped the timer. FALSE - if the timer was stopped before us.

}

// ret - useconds.
uint64 apic_timer_get_time_ellapsed() {
	APICRegisters * apic = g_apic_context.regs;
	spin_lock(&g_apic_context.is_timer_run_lock);
	uint64 ret = g_apic_context.bus_freq *1000 * 1000 / (apic->initial_count - apic->current_count);
	spin_unlock(&g_apic_context.is_timer_run_lock);
	return ret;
}
BOOL apic_timer_is_run() {
	return g_apic_context.is_timer_run;
}
typedef void(*APICTimerCallback)();
BOOL apic_timer_set_callback_function(APICTimerCallback cb) {

}
////


BOOL set_apic_timer(uint32 timer_us) {



}



BOOL init_APIC(char apic_address_for_ipi) {
	APICContext * apic_context = &g_apic_context;
	APICRegisters * apic = g_apic_context.regs;
	// init the local apic address for this LAPIC:
	apic->Destination_format = 0xffffffff; // means FLAT mode.
	apic->local_destination = apic->local_destination & 0x00FFFFFF | (apic_address_for_ipi << 24);

	// Disable all the LVTs:
	apic->LVT_CMCI = 0x10000; // Mask it
	apic->LVT_LINT0 = 0x10000; // Mask it
	apic->LVT_LINT1 = 0x10000; // Mask it
	apic->LVT_error = 0x10000; // Mask it
	apic->LVT_performance = 0x10000; // Mask it
	apic->LVT_thermal = 0x10000; // Mask it
	apic->LVT_timer = 0x10000; // Mask it

	//don't inhibit interrupts for now.
	apic->TPR = 0; // make task priority to zero. That means that we allow every interrupt priority.

	// Program the spurious interrupt, and permit the software to enable/disable the APIC:
	apic->Spurious_interrupt_vector = APIC_SPURIOUS | 0x10000;

	// Now configure the APIC timer to the rellevant ISR and make it in ONE_SHOT mode, and non-mask.
	apic->LVT_timer = APIC_TIMER;

	apic->divide_conf = 3; // divide by 16. Note that if the bus freq is ~ the cpu freq, so it will take about 0xffffffff * 16 opcodes to fire the timer.
						   //               Also, if this freq is < 10Ghz, it will take at least 1.6 seconds to fire the timer.
						   //				The point is, that this timer will never fire, because ~10ms from here we will stop it.

	////////////////////////////////////////////////////////////////
	// We want to measure how many ticks is 1 ms. To do so we will use the PIT clock, that run in predefined freq. of 1193180 Hz.
	//
	//
	// We will use PIT channel 2, because we can check when the timer is fire. Note that this channel control the speaker too.
	__out8(0x61, __in8(0x61) & 0b11111101); // zero bit 1 to shut the speaker up.
	
	// configure the channel:
	__out8(0x43, 0b10110010); // Binary mode (0), One-shot mode (001), 16bit mode (11), channel2 (10).

	// set the reload value (1193180 / 100 Hz = 11931 = 2e9bh, beacuse we want to measure 1 ms.
	__out8(0x42, 0x9b); 
	io_wait();
	__out8(0x42, 0x2e);

	// Start the counting (In the one-shot mode, we need to put 0 at the output bit and then 1.
	char orig = __in8(0x61);
	__out8(0x61, orig & 0b11111110);
	io_wait();
	__out8(0x61, orig | 0b00000001);

	//
	//
	////////////////////////////////////////////////////////////////

	// Reset the APIC timer:
	apic->initial_count = 0xffffffff;

	// Wait until the on-shot PIT will fire:
	while (!(__in8(0x61) & 0b00100000));

	// Stop the APIC timer:
	apic->LVT_timer = 0x10000; // Mask it

	// Calculate the BUS frequency:
	uint32 counts_elapsed = 0xffffffff - apic->current_count;
	g_apic_context.bus_freq = counts_elapsed * 16 * 100; // * 16 because we divide the APIC by 16. * 100 because we measure just 10ms.
												 // g_apic_context.bus_freq = number of ticks per second.
	
	//spin_init(g_is_timer_run_lock);
	//set_apic_timer(timer_ms);
	
		
	return TRUE;
}

BOOL enable_APIC() {
	if (__rdmsr(IA32_APIC_BASE_MSR) & 0x800 == 0) return FALSE;
	return TRUE;
}

BOOL enable_interrupts() {
	__sti();
}