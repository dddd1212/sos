#ifndef PROCESSOR_SCHEDULER_INFO
#define PROCESSOR_SCHEDULER_INFO
#include "../Common/Qube.h"

typedef QResult(*SystemTaskFunction)(void* arg);
struct SYSTEM_TASK {
	SystemTaskFunction func;
	void* arg;
	struct SYSTEM_TASK* next;
};

typedef struct SYSTEM_TASK SYSTEM_TASK;

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
	SYSTEM_TASK* first_system_task;
	SYSTEM_TASK* last_system_task;
}SchedulerInfo;
#endif