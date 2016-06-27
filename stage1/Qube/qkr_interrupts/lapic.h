#ifndef __APIC_H_
#define __APIC_H_
#include "../Common/Qube.h"
#define LAPIC_REG_BASE 0xfee00000

#define IA32_APIC_BASE_MSR 0x1b

// old PIC consts:
#define PIC1_COMMAND	0x20
#define PIC1_DATA		0x21
#define PIC2_COMMAND	0xa0
#define PIC2_DATA		0xa1

#define PIT_FREQ 1193182
///

#define LAPIC_DWORD(x) uint32 x; \
					  char x##pad[12];



typedef struct {
	LAPIC_DWORD(reserved0); // 0x0
	LAPIC_DWORD(reserved10); // 0x10
	LAPIC_DWORD(local_apic_id); // 0x20
	LAPIC_DWORD(local_apic_version); // 0x30
	LAPIC_DWORD(reserved40); // 0x40
	LAPIC_DWORD(reserved50); // 0x50
	LAPIC_DWORD(reserved60); // 0x60
	LAPIC_DWORD(reserved70); // 0x70
	LAPIC_DWORD(TPR); // 0x80 - Task Priority Register
	LAPIC_DWORD(APR); // 0x90 - Arbitration Priority Register
	LAPIC_DWORD(PPR); // 0xa0 - Processor Priority Register
	LAPIC_DWORD(EOI); // 0xb0 - End Of Interrupt Register
	LAPIC_DWORD(RRD); // 0xc0 - Remote Read Register
	LAPIC_DWORD(local_destination); // 0xd0
	LAPIC_DWORD(Destination_format); // 0xe0
	LAPIC_DWORD(Spurious_interrupt_vector); // 0xf0
	LAPIC_DWORD(ISR0); // 0x100
	LAPIC_DWORD(ISR1); // 0x110
	LAPIC_DWORD(ISR2); // 0x120
	LAPIC_DWORD(ISR3); // 0x130
	LAPIC_DWORD(ISR4); // 0x140
	LAPIC_DWORD(ISR5); // 0x150
	LAPIC_DWORD(ISR6); // 0x160
	LAPIC_DWORD(ISR7); // 0x170
	LAPIC_DWORD(TMR0); // 0x180
	LAPIC_DWORD(TMR1); // 0x190
	LAPIC_DWORD(TMR2); // 0x1a0
	LAPIC_DWORD(TMR3); // 0x1b0
	LAPIC_DWORD(TMR4); // 0x1c0
	LAPIC_DWORD(TMR5); // 0x1d0
	LAPIC_DWORD(TMR6); // 0x1e0
	LAPIC_DWORD(TMR7); // 0x1f0
	LAPIC_DWORD(IRR0); // 0x200
	LAPIC_DWORD(IRR1); // 0x210
	LAPIC_DWORD(IRR2); // 0x220
	LAPIC_DWORD(IRR3); // 0x230
	LAPIC_DWORD(IRR4); // 0x240
	LAPIC_DWORD(IRR5); // 0x250
	LAPIC_DWORD(IRR6); // 0x260
	LAPIC_DWORD(IRR7); // 0x270
	LAPIC_DWORD(error_status); // 0x280
	LAPIC_DWORD(reserved290); // 0x290
	LAPIC_DWORD(reserved2a0); // 0x2a0
	LAPIC_DWORD(reserved2b0); // 0x2b0
	LAPIC_DWORD(reserved2c0); // 0x2c0
	LAPIC_DWORD(reserved2d0); // 0x2d0
	LAPIC_DWORD(reserved2e0); // 0x2e0
	LAPIC_DWORD(LVT_CMCI); // 0x2f0
	LAPIC_DWORD(ICR_low); // 0x300
	LAPIC_DWORD(ICR_high); // 0x310
	LAPIC_DWORD(LVT_timer); // 0x320
	LAPIC_DWORD(LVT_thermal); // 0x330
	LAPIC_DWORD(LVT_performance); // 0x340
	LAPIC_DWORD(LVT_LINT0); // 0x350
	LAPIC_DWORD(LVT_LINT1); // 0x360
	LAPIC_DWORD(LVT_error); // 0x370
	LAPIC_DWORD(initial_count); // 0x380
	LAPIC_DWORD(current_count); // 0x390
	LAPIC_DWORD(reserved3a0); // 0x3a0
	LAPIC_DWORD(reserved3b0); // 0x3b0
	LAPIC_DWORD(reserved3c0); // 0x3c0
	LAPIC_DWORD(reserved3d0); // 0x3d0
	LAPIC_DWORD(divide_conf); // 0x3e0
	LAPIC_DWORD(reserved3f0); // 0x3f0
} LAPICRegisters;

// This function must be call before every other lapic_XXX functions.
// TODO: get the address alone by allocating the physical memory.
void lapic_init(LAPICRegisters * regs);

// return the current processor id.
EXPORT uint8 lapic_get_this_processor_index();

// Enable and start the local-apic functionality at the current processor.
BOOL lapic_start();

// Checks if the LAPIC and the IOAPIC are present in the hardware.
BOOL _is_LAPIC_exists();

// Configure the legacy PIC to be disabled except from the timer.
BOOL _configure_old_PIC();

// Configure the LAPIC
BOOL _configure_LAPIC();

// enable it
BOOL _enable_LAPIC();

// global variable that points to the local apic registers.
extern EXPORT LAPICRegisters * g_lapic_regs;


/************* LAPIC TIMER ***********
These functions are the api for the lapic timer.
Every processor in the system has a timer that can be controlled through these functions.
To use a processor timer, you first need to register callback function that will be called in the context
of the timer interrupt when the timer will fire.
Then you need to call to lapic_start_timer to start the timer with timer_us.
Every timer-api function can be called from the callback function.
**************************************/

// This function init the current processor lapic timer.
// To init the timer you have to run in the context of the target processor (Design decision. not architecture mandatory).
BOOL _lapic_timer_init(); // Should be called only once before any calls to the lapic_timer api.



// lapic_timer api:

// returns TRUE iff the current processor timer is initialized.
//
EXPORT BOOL lapic_is_timer_inited();

// Starts the current processor timer.
// timer_us - the time until the callback function is called, in micro-seconds. (see lapic_timer_set_callback_function).
// is_periodic - is the timer will be periodic or one shot mode.
// Note: timer_us has a limit that can be returned by lapic_timer_get_max_wait_time.
//
// return value - QSuccess on success. QFailure on failure.
EXPORT QResult lapic_timer_start(uint64 timer_us, BOOL is_periodic);

// Stops the current processor timer.
// time_ellapsed (OUT) - when the function succeed, if time_ellapsed is not NULL, it will filled with the time ellapsed since the success call to the lapic_timer_start.
//
// return value - QSuccess on success. QFailure on failure.
EXPORT QResult lapic_timer_stop(uint64 * time_ellapsed);

// Returns the maximum valid wait time for the timer.
EXPORT uint64 lapic_timer_get_max_wait_time();

// Gets the time ellapsed since the last succeed call to start_timer for the current processor.
// time_ellapsed (OUT) - when the function succeed, time_ellapsed, that should not be NULL, will filled with the time ellapsed since the success call to the lapic_timer_start.
//
// return value - QSuccess on success. QFailure on failure.
EXPORT QResult lapic_timer_get_time_ellapsed(uint64 * time_ellapsed);

// The function returns TRUE iff the current processor timer is in run state (thats means that lapic_start_timer succeed for the processor_id, and the timer not stops yet.
EXPORT BOOL lapic_timer_is_run();

typedef void(*APICTimerCallback)();

// Register the function that will be called when the timer will be fire.
// All the processors must have the same callback function (Design decision. not architecture mandatory).
//
// return value - QSuccess on success. QFailure on failure.
EXPORT QResult lapic_timer_set_callback_function(APICTimerCallback cb);



#endif // __APIC_H_
