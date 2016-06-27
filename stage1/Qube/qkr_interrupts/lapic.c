#include "lapic.h"
#include "interrupts.h"
#include "processors.h"
#include "../Common/Qube.h"
#include "../Common/spin_lock.h"

LAPICRegisters * g_lapic_regs; // The pointer to the lapic registers for the current processor. For all the processor its the same address, because its not real address, but io-mapped physical address.
uint64 g_bus_freq = 0; // 0 means uninitialized yet.

void lapic_init(LAPICRegisters * regs) {
	g_lapic_regs = regs;
	return;
}


uint8 lapic_get_this_processor_index() {
	return g_lapic_regs->local_apic_id;
}

BOOL lapic_start() {
	return (
		// First thing we want to validate that we support the hardware
		_is_LAPIC_exists() &&

		// Configure the legacy PIC to be disabled except from the timer.
		_configure_old_PIC() &&

		// Configure the LAPIC
		_configure_LAPIC() &&

		// Now we ready to enable the APIC
		_enable_LAPIC() &&

		_lapic_timer_init()
		);
}

BOOL _is_LAPIC_exists() {
	// Check if LAPIC exist:
	int code = 1;
	uint32 eax_out;
	uint32 edx_out;
	__cpuid(code, &eax_out, &edx_out);
	if (!(edx_out & (1 << 9))) return FALSE;
	// Validate that the base address of the APIC registers are as expected:

	// TODO: the field is more then 32bit. we need to see what the size and & with these bits.
	if ((__rdmsr(IA32_APIC_BASE_MSR) & 0xfffff000) != LAPIC_REG_BASE) return FALSE;

	return TRUE;
}

BOOL _configure_old_PIC() {
	// Configure the timer (PIT):
	//__out8(0x43, 0x36); // We going to configure channel 0.
	//__out8(0x40, 0x1); // Low byte
	//__out8(0x40, 0x0); // High byte

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
	return TRUE;
}

BOOL _configure_LAPIC() {
	LAPICRegisters * apic = g_lapic_regs;
	// init the local apic address for this LAPIC:
	apic->Destination_format = 0xffffffff; // means FLAT mode.
	
	//TODO: check if we need it.
	//apic->local_destination = apic->local_destination & 0x00FFFFFF | (apic_address_for_ipi << 24);

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
	apic->Spurious_interrupt_vector = APIC_SPURIOUS | 0x100;

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
	
	// set the reload value (1193180 / 100 Hz = 11931 = 2e9bh, beacuse we want to measure 10 ms.
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
	apic->LVT_timer |= 0x10000; // Mask it
	
	// Stop the PIT:
	__out8(PIC1_DATA, 0xff); // mask everything

	// Calculate the BUS frequency:
	uint32 counts_elapsed = 0xffffffff - apic->current_count;
	g_bus_freq = counts_elapsed * 16 * 100; // * 16 because we divide the APIC by 16. * 100 because we measure just 10ms.
																		  // g_bus_freq = number of ticks per second.
	uint64 ggg = g_bus_freq;

	return TRUE;
}

BOOL _enable_LAPIC() {
	if ((__rdmsr(IA32_APIC_BASE_MSR) & 0x800) == 0) return FALSE;
	return TRUE;
}

// Timer api //

BOOL _lapic_timer_init() {
	ProcessorControlBlock * pcb = this_processor_control_block();
	if (pcb->timer_inited) return FALSE; // already inited.
	if (g_bus_freq == 0) return FALSE; // bus_freq not calc yet.
	pcb->is_timer_run = FALSE; // the timer is in stop state.
	spin_init(&pcb->timer_lock); // init te lock.
	pcb->timer_inited = TRUE; // mark inited.
	return TRUE;
}

BOOL lapic_is_timer_inited() {

	return this_processor_control_block()->timer_inited;
}

QResult lapic_timer_start(uint64 timer_us, BOOL is_periodic) {
	ProcessorControlBlock * pcb = this_processor_control_block();
#ifdef DEBUG
	if (pcb->timer_inited == 0) return FALSE;
#endif
	// Calculate how many ticks we need:
	uint64 ticks = g_bus_freq * timer_us / 1000 / 1000;
	uint32 divide_number = 1;
	uint32 timer_mode = 0; // one shot mode
	if (is_periodic) timer_mode = 0x20000;  // periodic mode
	char divide_value = 0b11111111; // we will use only the 4 low bits.

	while (ticks > 0xffffffff) {
		divide_number *= 2;
		divide_value += 1; // we will use only 4 low bits.
		if (divide_value == 0b100) {
			divide_value = 0b1000; // The 3rd bit must be zero (not part of the divide value)...
		}
		ticks /= 2;
		if (divide_number > 128) { // We can't divide more then 128..
			return FALSE; // time too long to wait it in the timer.
		}
	}


	spin_lock(&pcb->timer_lock);
	BOOL ret = TRUE;
	do { // Locked
		//if (pcb->is_timer_run) {
		//	ret = FALSE;
		//	break;
		//}
		pcb->is_timer_run = TRUE;
		pcb->cur_divide_number = divide_number;
		g_lapic_regs->divide_conf = divide_value & 0b1011;
		g_lapic_regs->initial_count = (uint32)ticks;
		g_lapic_regs->LVT_timer = timer_mode | (g_lapic_regs->LVT_timer & 0xff); // unmask the interrupt. one-shot mode.
	} while (0);
	spin_unlock(&pcb->timer_lock);
	return ret;
}

QResult lapic_timer_stop(uint64 * time_ellapsed) {
	ProcessorControlBlock * pcb = this_processor_control_block();
#ifdef DEBUG
	if (pcb->timer_inited == 0) return FALSE;
#endif
	BOOL ret;
	spin_lock(&pcb->timer_lock);
	do { // Locked
		g_lapic_regs->LVT_timer |= 0x10000; // mask the interrupt.
		ret = pcb->is_timer_run;
		pcb->is_timer_run = FALSE;
		if (time_ellapsed) {
			*time_ellapsed = (g_lapic_regs->initial_count - g_lapic_regs->current_count) * pcb->cur_divide_number * 1000 * 1000 / g_bus_freq;
		}
	} while (0);
	spin_unlock(&pcb->timer_lock);
	return ret; // TRUE - if we stopped the timer. FALSE - if the timer was stopped before us.

}

uint64 lapic_timer_get_max_wait_time() {
	if (g_bus_freq == 0) return 0;
	// We can wait maximum time of 128 * 0xffffffff ticks:
	return 128 * 0xffffffff * 1000 * 1000 / g_bus_freq;
}


QResult lapic_timer_get_time_ellapsed(uint64 * time_ellapsed) {
	ProcessorControlBlock * pcb = this_processor_control_block();
#ifdef DEBUG
	if (pcb->timer_inited == 0) return FALSE;
#endif
	spin_lock(&pcb->timer_lock);
	*time_ellapsed = (g_lapic_regs->initial_count - g_lapic_regs->current_count) * pcb->cur_divide_number * 1000 * 1000 / g_bus_freq;
	spin_unlock(&pcb->timer_lock);
	return TRUE;
}
BOOL lapic_timer_is_run() {
	return this_processor_control_block()->is_timer_run;
}

BOOL lapic_timer_set_callback_function(APICTimerCallback cb) {
	this_processor_control_block()->timer_callback = cb;
	return TRUE;
}
////