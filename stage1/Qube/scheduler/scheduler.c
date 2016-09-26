#include "scheduler.h"
#include "../screen/screen.h"
#include "../libc/string.h"
#include "../MemoryManager/memory_manager.h"
#include "../Common/spin_lock.h"
#include "../Common/intrinsics.h"

SpinLock thread_selecting_lock;
ThreadBlock* pop_next_thread();

ThreadBlock *first_thread, *last_thread;

void push_swap_pop() __attribute__((noinline));
void update_cur_and_prev_threads(ThreadBlock* new_thread);
void finish_thread_swap_out();

void idle() {
	ThreadBlock* current_thread;
	current_thread = ((ProcessorBlock*)__get_processor_block())->scheduler_info.cur_thread;
	while ((first_thread==NULL) && (current_thread->thread_status.running_state!=RUN)); // TODO: halt.
}

void thread_begin(ThreadStartFunction start_addr) {
	start_addr();
}


QResult qkr_main(KernelGlobalData * kgd) {
	first_thread = last_thread = NULL; //TODO: is that needed?
	spin_init(&thread_selecting_lock);
	return QSuccess;
}

QResult schedule_next() {
	ThreadBlock* new_thread, *current_thread;
	current_thread = ((ProcessorBlock*)__get_processor_block())->scheduler_info.cur_thread;
	do {
		new_thread = pop_next_thread();
		if (new_thread == NULL) {
			// check if we can resume current thread and if so, just return.
			if (current_thread->thread_status.running_state == RUN) {
				return QSuccess;
			}
			idle();
		}
	} while (new_thread == NULL);
	uint64 new_stack = new_thread->thread_status.RSP;
	/* push the status to the current stack, then swap to the new stack and pop the new status from there.
	 * usually, the function push_swap_pop will return to here with the context of the new thread, where he stop last time.
	 * in the case where the new thread runs for the first time, push_swap_pop will return to thread_begin function because 
	 * when it was created with "fake" stack in create_new_thred.
	*/
	update_cur_and_prev_threads(new_thread);
	push_swap_pop(new_stack, &current_thread->thread_status.RSP, new_thread);
	finish_thread_swap_out();
	return QSuccess;
}

void push_swap_pop(uint64 new_stack, uint64* old_stack) {
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
}

ThreadBlock* pop_next_thread() {
	ThreadBlock *selected_thread;
	spin_lock(&thread_selecting_lock);
	if (first_thread) {
		selected_thread = first_thread;
		first_thread = selected_thread->next;
		if (first_thread == NULL) {
			last_thread = NULL;
		}
	}
	else {
		selected_thread = NULL;
	}
	spin_unlock(&thread_selecting_lock);
	return selected_thread;
}

void update_cur_and_prev_threads(ThreadBlock* new_thread) {
	ProcessorBlock* processor_block = (ProcessorBlock*)__get_processor_block();
	processor_block->scheduler_info.prev_thread = processor_block->scheduler_info.cur_thread;
	processor_block->scheduler_info.cur_thread = new_thread;
}

void finish_thread_swap_out() {
	ThreadBlock* old_thread;
	ProcessorBlock* processor_block = (ProcessorBlock*)__get_processor_block();
	old_thread = processor_block->scheduler_info.prev_thread;
	switch (old_thread->thread_status.running_state) {
	case RUN:
		// TODO: insert to running queue
		break;
	case WAIT:
		// TODO: insert to waiting queue
		break;
	case KILLED:
		// TODO: free the ThreadBlock
		break;
	}
	// TODO: insert to
}