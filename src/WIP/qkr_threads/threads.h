#ifndef __threads_h__
#define __threads_h__
#include "../Common/Qube.h"
#include "../QbjectManager/Qbject.h"
typedef int32 TID;
typedef enum {
	READY = 1, // The thread ready to run
	RUNNING = 2, // The thread is running now.
	WAITING = 3, // Wait state.
	EXITING = 4, // thread finished.
} ThreadState;


typedef struct {
	TID tid;
	void * stack;
	ThreadContext con;
} ThreadQNode;


Qbject thread_create();
QResult thread_suspend(TID tid);
QResult thread_resume(TID tid);
QResult thread_kill(TID tid);

TID thread_get_id(Qbject * thread);


context_switch();



#endif // __threads_h__

