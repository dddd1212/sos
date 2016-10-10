#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "../Common/Qube.h"




typedef void(*ThreadStartFunction)(void);


typedef enum {
	RUNNING,
	READY,
	WAITING,
	KILLED
} RunningState;

typedef struct {
	uint64 RSP;
	RunningState running_state;
} ThreadStatus;

typedef struct ThreadBlock ThreadBlock;
struct ThreadBlock {
	ThreadStatus thread_status;
	ThreadBlock* next;
	ThreadBlock* prev;
};

typedef struct {
	ThreadBlock* cur_thread;
	ThreadBlock* prev_thread;
	RunningState prev_thread_new_state;
	ThreadBlock* idle_thread;
}SchedulerInfo;



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


#include "../qkr_interrupts/interrupts.h"
#include "../qkr_interrupts/processors.h"
#endif
