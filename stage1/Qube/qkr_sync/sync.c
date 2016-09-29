#include "sync.h"
#include "../MemoryManager/memory_manager.h"
#include "../Common/spin_lock.h"
#include "../scheduler/scheduler.h"
typedef struct Waiter Waiter;
struct Waiter {
	ThreadBlock *thread;
	Waiter* next;
};
typedef struct {
	SpinLock lock;
	BOOL is_set;
	Waiter* first_waiter;
	Waiter* last_waiter;
}EventInternal;

QResult create_event(Event* p_event) {
	EventInternal* event;
	event = (EventInternal*) kheap_alloc(sizeof(EventInternal));
	event->is_set = FALSE;
	spin_init(&event->lock);
	event->first_waiter = NULL;
	event->last_waiter = NULL;
	*p_event = (Event)event;
	return QSuccess;
}

QResult wait_for_event(Event a_event) {
	EventInternal* event = (EventInternal*)a_event;
	if (event->is_set) {
		return QSuccess;
	}
	Waiter* waiter = (Waiter*)kheap_alloc(sizeof(Waiter));
	waiter->thread = get_current_thread_block();
	waiter->next = NULL;
	spin_lock(&event->lock);
	if (event->first_waiter) {
		event->last_waiter->next = waiter;
	}
	else {
		event->first_waiter = event->last_waiter = waiter;
	}
	spin_unlock(&event->lock);
}

QResult set_event(Event a_event) {
	EventInternal* event = (EventInternal*)a_event;
	Waiter* waiter;
	spin_lock(&event->lock);
	waiter = event->first_waiter;
	event->first_waiter = NULL;
	event->last_waiter = NULL;
	spin_unlock(&event->lock);
	while (waiter) {
		while(waiter->thread->thread_status.running_state == RUNNING); // busy wait in case the thread has registered to the event, but didn't swaped out yet.
		set_thread_as_ready(waiter->thread);
		waiter = waiter->next;
	}
}

QResult delete_event(Event p_event) {
	kheap_free(p_event);
	return QSuccess;
}

QResult qkr_main(KernelGlobalData * kgd) {
	return QSuccess;
}

QResult wait_for_event(Event a_event) {
	EventInternal* event = (EventInternal*)a_event;
	if (((Qbject*)event)->associated_qnode != )
}