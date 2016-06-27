#ifndef __Processor_h__
#define __Processor_h__

#include "../Common/Qube.h"
#include "../Common/intrinsics.h"
#include "lapic.h"
#define MAX_NUM_OF_PROCESSORS 15
typedef struct {
	LAPICRegisters * regs; // This field is just for convinence: It had to be APIC_REG_BASE for all of the processors.
						   // There is no different in accessing the regs through this field or implicit by the APIC_REG_BASE.
	uint64 bus_freq;
	
	BOOL timer_inited; // is init
	BOOL is_timer_run; // is running (not stop)
	uint64 cur_divide_number; // the current divide number. saved to imporve running time.
	SpinLock timer_lock; // used to sync timer operations.
	APICTimerCallback timer_callback;
} ProcessorControlBlock;

#define GS_OFFSET_PROCESSOR_CONTROL_BLOCK_PTR 0
typedef struct {
	ProcessorControlBlock * pcb;
	uint64 padding;
} GS;



BOOL init_processors_data();

extern EXPORT ProcessorControlBlock g_pcbs[MAX_NUM_OF_PROCESSORS];

static inline ProcessorControlBlock * this_processor_control_block() __attribute__((always_inline));
static inline ProcessorControlBlock * this_processor_control_block() {
	//ProcessorControlBlock * ret;
	//__READ_FROM_GS((uint64)ret, GS_OFFSET_PROCESSOR_CONTROL_BLOCK_PTR);
	return &g_pcbs[lapic_get_this_processor_index()];
	//return ret;
}


void set_gs_base(GS * base);



ProcessorControlBlock * get_processor_control_block(uint8 idx);

QResult init_this_processor_control_block();



#endif // __Processor_h__

