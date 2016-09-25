#ifndef QBJECT_H
#define QBJECT_H
#include "../Common/Qube.h"
typedef void(*ThreadStartFunction)(void);


typedef enum {
	RUN,
	WAIT,
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
}SchedulerInfo;



// this part probably should be moved to somewhere else.
typedef struct ProcessorBlock ProcessorBlock;
struct ProcessorBlock {
	ProcessorBlock* pointer_to_self;
	SchedulerInfo scheduler_info;
};
#endif
