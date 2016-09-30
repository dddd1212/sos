#include "scheduler.h"
#include "../screen/screen.h"
#include "../libc/string.h"
#include "../MemoryManager/memory_manager.h"
#include "../Common/spin_lock.h"
#include "../Common/intrinsics.h"

#define NEW_THREAD_STACK_SIZE 0x8000

ThreadBlock* pop_next_thread();

SpinLock ready_list_lock;
ThreadBlock *first_ready_thread, *last_ready_thread;

SpinLock waiting_list_lock;
ThreadBlock *first_waiting_thread, *last_waiting_thread;

void push_swap_pop() __attribute__((noinline));
void update_cur_and_prev_threads(ThreadBlock* new_thread, RunningState prev_thread_new_state);
void finish_thread_swap_out();
void init_this_processor_idle_thread();
void push_swap_pop(uint64 new_stack, uint64* old_stack);
ThreadBlock* get_this_processor_idle_thread();

void idle() {
	while (first_ready_thread == NULL); // TODO: halt.
	schedule_next(READY);
}

void thread_begin(ThreadStartFunction start_addr) {
	finish_thread_swap_out();
	start_addr();
}

ThreadBlock* get_current_thread_block() {
	return ((ProcessorBlock*)__get_processor_block())->scheduler_info.cur_thread;
}

QResult qkr_main(KernelGlobalData * kgd) {
	init_this_processor_idle_thread();
	first_ready_thread = last_ready_thread = NULL;//get_this_processor_idle_thread();
	//first_ready_thread->next = first_ready_thread->prev = NULL;
	first_waiting_thread = last_waiting_thread = NULL; //TODO: is that needed?
	spin_init(&ready_list_lock);
	spin_init(&waiting_list_lock);
	ProcessorBlock* processor_block = (ProcessorBlock*)__get_processor_block();
	ThreadBlock* this_thread = (ThreadBlock*)kheap_alloc(sizeof(ThreadBlock));
	this_thread->next = this_thread->prev = NULL;
	this_thread->thread_status.running_state = RUNNING;
	this_thread->thread_status.RSP = 0;
	processor_block->scheduler_info.cur_thread = this_thread;
	//schedule_next();
	return QSuccess;
}

QResult schedule_next(RunningState old_thread_new_state) {
	ThreadBlock* new_thread, *current_thread;
	current_thread = get_current_thread_block();
	do {
		new_thread = pop_next_thread();
		if (new_thread == NULL) {
			// check if we can resume current thread and if so, just return.
			if (old_thread_new_state == READY) {
				return QSuccess;
			}
			new_thread = get_this_processor_idle_thread();
		}
	} while (new_thread == NULL);
	uint64 new_stack = new_thread->thread_status.RSP;
	/* push the status to the current stack, then swap to the new stack and pop the new status from there.
	 * usually, the function push_swap_pop will return to here with the context of the new thread, where he stop last time.
	 * in the case where the new thread runs for the first time, push_swap_pop will return to thread_begin function because 
	 * when it was created with "fake" stack in create_new_thred.
	*/
	update_cur_and_prev_threads(new_thread, old_thread_new_state);
	push_swap_pop(new_stack, &current_thread->thread_status.RSP);
	finish_thread_swap_out();
	return QSuccess;
}

/*void push_swap_pop(uint64 new_stack, uint64* old_stack) {
	__asm__(
		".intel_syntax noprefix;"
		"push rax;"
		"push rbx;"
		"push rcx;"
		"push rdx;"
		"push r8;"
		"push r9;"
		"push r10;"
		"push r11;"
		"push r12;"
		"push r13;"
		"push r14;"
		"push r15;"
		"mov [rsi],rsp;" // save the old stack
		"mov rsp,rdi;"   // and load the new stack
		"pop r15;"
		"pop r14;"
		"pop r13;"
		"pop r12;"
		"pop r11;"
		"pop r10;"
		"pop r9;"
		"pop r8;"
		"pop rdx;"
		"pop rcx;"
		"pop rbx;"
		"pop rax;"
		".att_syntax;"
		: 
		: "D"(new_stack), "S"(old_stack)
	);
}*/

ThreadBlock* pop_next_thread() {
	ThreadBlock *selected_thread;
	spin_lock(&ready_list_lock);
	if (first_ready_thread) {
		selected_thread = first_ready_thread;
		first_ready_thread = selected_thread->next;
		if (first_ready_thread == NULL) {
			last_ready_thread = NULL;
		}
		else {
			first_ready_thread->prev = NULL;
		}
	}
	else {
		selected_thread = NULL;
	}
	spin_unlock(&ready_list_lock);
	return selected_thread;
}

void update_cur_and_prev_threads(ThreadBlock* new_thread, RunningState prev_thread_new_state) {
	ProcessorBlock* processor_block = (ProcessorBlock*)__get_processor_block();
	processor_block->scheduler_info.prev_thread = processor_block->scheduler_info.cur_thread;
	processor_block->scheduler_info.prev_thread_new_state = prev_thread_new_state;
	processor_block->scheduler_info.cur_thread = new_thread;
}

void finish_thread_swap_out() {
	ThreadBlock* old_thread;
	ProcessorBlock* processor_block = (ProcessorBlock*)__get_processor_block();
	old_thread = processor_block->scheduler_info.prev_thread;
	switch (processor_block->scheduler_info.prev_thread_new_state) {
	case READY:
		// TODO: insert to running queue and mark as READY. also handle the case of idle_thread
		if (old_thread == get_this_processor_idle_thread()) {
			return;
		}
		spin_lock(&ready_list_lock);
		old_thread->thread_status.running_state = READY;
		if (first_ready_thread == NULL) {
			first_ready_thread = last_ready_thread = old_thread;
			old_thread->next = old_thread->prev = NULL;
		}
		else {
			old_thread->prev = last_ready_thread;
			last_ready_thread->next = old_thread;
			old_thread->next = NULL;
			last_ready_thread = old_thread;
		}
		spin_unlock(&ready_list_lock);
		break;
	case WAITING:
		spin_lock(&waiting_list_lock);
		old_thread->thread_status.running_state = WAITING;
		if (first_waiting_thread == NULL) {
			first_waiting_thread = last_waiting_thread = old_thread;
			old_thread->next = old_thread->prev = NULL;
		}
		else {
			old_thread->prev = last_waiting_thread;
			last_waiting_thread->next = old_thread;
			old_thread->next = NULL;
			last_waiting_thread = old_thread;
		}
		spin_unlock(&waiting_list_lock);
		break;
	case KILLED:
		// TODO: free the ThreadBlock
		break;
	default:
		// suicide
		break;
	}
	// TODO: insert to
}

QResult set_thread_as_ready(ThreadBlock* thread) {
	spin_lock(&waiting_list_lock);
	ThreadBlock *prev, *next;
	prev = thread->prev;
	next = thread->next;
	if (prev) {
		prev->next = thread->next;
	}
	else {
		first_ready_thread = next;
	}
	if (next) {
		next->prev = thread->prev;
	}
	else{
		last_ready_thread = prev;
	}
	spin_unlock(&waiting_list_lock);
	
	spin_lock(&ready_list_lock);
	if (first_ready_thread) {
		last_ready_thread->next = thread;
		thread->prev = last_ready_thread;
		thread->next = NULL;
		last_ready_thread = thread;
	}
	else {
		thread->next = thread->prev = NULL;
		first_ready_thread = last_ready_thread = thread;
	}
	spin_unlock(&ready_list_lock);
	return QSuccess;
}

typedef struct {
	uint64 rbp;
	uint64 rdi;
	uint64 rsi;
	uint64 r15;
	uint64 r14;
	uint64 r13;
	uint64 r12;
	uint64 r11;
	uint64 r10;
	uint64 r9;
	uint64 r8;
	uint64 rdx;
	uint64 rcx;
	uint64 rbx;
	uint64 rax;
	uint64 ret;
} FakeStack;

ThreadBlock* create_new_thread_block(ThreadStartFunction start_addr) {
	ThreadBlock* thread_block = (ThreadBlock*) kheap_alloc(sizeof(ThreadBlock));
	void* stack = kheap_alloc(NEW_THREAD_STACK_SIZE);
	FakeStack* fake_stack = (FakeStack*)(((uint64)stack + NEW_THREAD_STACK_SIZE) - sizeof(FakeStack));
	fake_stack->rdi = (uint64)start_addr;
	fake_stack->ret = (uint64)thread_begin;
	thread_block->thread_status.RSP = (uint64)fake_stack;
	thread_block->thread_status.running_state = READY;
	return thread_block;
}

QResult start_new_thread(ThreadStartFunction start_addr) {
	ThreadBlock* new_thread = create_new_thread_block(start_addr);
	spin_lock(&ready_list_lock);
	if (first_ready_thread == NULL) {
		first_ready_thread = last_ready_thread = new_thread;
		new_thread->next = new_thread->prev = NULL;
	}
	else {
		new_thread->prev = last_ready_thread;
		last_ready_thread->next = new_thread;
		new_thread->next = NULL;
		last_ready_thread = new_thread;
	}
	spin_unlock(&ready_list_lock);
	return QSuccess;
}

void init_this_processor_idle_thread() {
	ThreadBlock* idle_thread = create_new_thread_block(idle);
	ProcessorBlock* processor_block = (ProcessorBlock*)__get_processor_block();
	processor_block->scheduler_info.idle_thread = idle_thread;
}

ThreadBlock* get_this_processor_idle_thread() {
	return ((ProcessorBlock*)__get_processor_block())->scheduler_info.idle_thread;
}