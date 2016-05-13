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
	// regular registers
	uint64 rax;
	uint64 rbx;
	uint64 rcx;
	uint64 rdx;
	uint64 rsi;
	uint64 rbp;
	uint64 r8;
	uint64 r9;
	uint64 r10;
	uint64 r11;
	uint64 r12;
	uint64 r13;
	uint64 r14;
	uint64 r15;

	uint64 rip;
	
	uint32 eflags;
	
	uint16 cs;
	uint16 ss;
	uint16 ds;
	uint16 es;
	uint16 fs;
	uint16 gs;

} ThreadContext;

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

