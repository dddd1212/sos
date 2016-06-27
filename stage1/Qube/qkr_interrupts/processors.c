#include "processors.h"
#include "interrupts.h"
#include "../MemoryManager/heap.h"
#include "../Common/intrinsics.h"
#include "../Common/spin_lock.h"
#include "../libc/string.h"

ProcessorControlBlock g_pcbs[MAX_NUM_OF_PROCESSORS];
BOOL init_processors_data() {
	memset(g_pcbs, 0, sizeof(g_pcbs));
	return TRUE;
}
void set_gs_base(GS * base) {
	__wrmsr(0xC0000101, (uint64)base);
}

ProcessorControlBlock * get_processor_control_block(uint8 idx) {
	if (idx == 0xff) return this_processor_control_block();
	return &g_pcbs[idx];
}

BOOL init_this_processor_control_block() {
	GS * gs;
	// Get this processor id:
	uint8 idx = lapic_get_this_processor_index();
	// put the g_pcb[id] in gs:
	gs = (GS*)kheap_alloc(0x10); // gs must be align by 0x10
	if (gs == NULL) return QFail;
	gs->pcb = &g_pcbs[idx];
	set_gs_base(gs);

	// fill the current pcb in data:
	ProcessorControlBlock * pcb = this_processor_control_block();
	pcb->bus_freq = 0;
	pcb->is_timer_run = FALSE;
	spin_init(&pcb->timer_lock);
	pcb->regs = g_lapic_regs;
	
	
	return TRUE;
}