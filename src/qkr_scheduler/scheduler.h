#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "../Common/Qube.h"

#include "processor_scheduler_info.h"
#include "../qkr_interrupts/interrupts.h"
#include "../qkr_interrupts/processors.h"



typedef void(*ThreadStartFunction)(void);



// this part probably should be moved to somewhere else.
//typedef struct ProcessorBlock ProcessorBlock;
/*struct ProcessorBlock {
	ProcessorBlock* pointer_to_self;
	SchedulerInfo scheduler_info;
};*/

EXPORT ThreadBlock* get_current_thread_block();
EXPORT QResult set_thread_as_ready(ThreadBlock* waiter);
EXPORT QResult start_new_thread(ThreadStartFunction start_addr);
EXPORT void schedule_next(RunningState current_thread_next_state);
EXPORT void disable_scheduling();
EXPORT void enable_scheduling();
EXPORT BOOL is_scheduling_enabled();
EXPORT QResult add_system_task(SystemTaskFunction func, void* arg);


#endif
