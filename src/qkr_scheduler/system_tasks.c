#include "scheduler.h"
#include "../qkr_interrupts/interrupts.h"
#include "../qkr_interrupts/processors.h"
#include "../qkr_memory_manager/heap.h"
QResult add_system_task(SystemTaskFunction func, void* arg) {
	SYSTEM_TASK* task = (SYSTEM_TASK*)kheap_alloc(sizeof(SYSTEM_TASK));
	task->arg = arg;
	task->func = func;
	task->next = NULL;
	disable_interrupts();
	if (this_processor_control_block()->scheduler_info.first_system_task) {
		this_processor_control_block()->scheduler_info.last_system_task->next = task;
		this_processor_control_block()->scheduler_info.last_system_task = task;
	}
	else {
		this_processor_control_block()->scheduler_info.first_system_task = this_processor_control_block()->scheduler_info.last_system_task = task;
	}
	enable_interrupts();
	issue_scheduler_interrupt();
	return QSuccess;
}