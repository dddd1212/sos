#include "Qube.h"
/************* LAPIC TIMER ***********
These functions are the api for the lapic timer.
Every processor in the system has a timer that can be controlled through these functions.
To use a processor timer, you first need to register callback function that will be called in the context
of the timer interrupt when the timer will fire.
Then you need to call to lapic_start_timer to start the timer with timer_us.
After the timer is fired, it will not fire again, until you call the lapic_start_timer again.
Every timer-api function can be called from the callback function.
**************************************/

// This function init the current processor lapic timer.
// To init the timer you have to run in the context of the target processor (Design decision. not architecture mandatory).
// THIS FUNCTION IS NOT PART OF THE API.
BOOL _lapic_timer_init(); // Should be called only once before any calls to the lapic_timer api.


// returns TRUE iff the processor_id processor timer is initialized.
// processor_id - the local_apic_id / processor_id (this is the same thing) of the target processor, or -1 for the current processor.
//
EXPORT BOOL lapic_is_timer_inited(uint32 processor_id);

// Starts a specific processor timer.
// timer_us - the time until the callback function is called, in micro-seconds. (see lapic_timer_set_callback_function).
// processor_id - the local_apic_id / processor_id (this is the same thing) of the target processor, or -1 for the current processor.
// 
// Note: timer_us has a limit that can be returned by lapic_timer_get_max_wait_time.
//
// return value - QSuccess on success. QFailure on failure.
EXPORT QResult lapic_timer_start(uint64 timer_us, uint32 processor_id);

// Stops a specific processor timer.
// processor_id - the local_apic_id / processor_id (this is the same thing) of the target processor, or -1 for the current processor.
// time_ellapsed (OUT) - when the function succeed, if time_ellapsed is not NULL, it will filled with the time ellapsed since the success call to the lapic_timer_start.
//
// return value - QSuccess on success. QFailure on failure.
EXPORT QResult lapic_timer_stop(uint32 processor_id, uint64 * time_ellapsed);

// Returns the maximum valid wait time for the timer.
EXPORT uint64 lapic_timer_get_max_wait_time();

// Gets the time ellapsed since the last succeed call to start_timer for the processor_id processor.
// processor_id - the local_apic_id / processor_id (this is the same thing) of the target processor, or -1 for the current processor.
// time_ellapsed (OUT) - when the function succeed, time_ellapsed, that should not be NULL, will filled with the time ellapsed since the success call to the lapic_timer_start.
//
// return value - QSuccess on success. QFailure on failure.
EXPORT QResult lapic_timer_get_time_ellapsed(uint32 processor_id, uint64 * time_ellapsed);

// The function returns TRUE iff the processor_id timer is in run state (thats means that lapic_start_timer succeed for the processor_id, and the timer not stops yet.
EXPORT BOOL lapic_timer_is_run(uint32 processor_id);

typedef void(*APICTimerCallback)();

// Register the function that will be called when the timer will be fire.
// All the processors must have the same callback function (Design decision. not architecture mandatory).
//
// return value - QSuccess on success. QFailure on failure.
EXPORT QResult lapic_timer_set_callback_function(APICTimerCallback cb);

