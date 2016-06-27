#include "kernel.h"
#include "../screen/screen.h"
#include "../Common/Qube.h"
#include "../MemoryManager/memory_manager.h"
#include "../QbjectManager/Qbject.h"
#include "../qkr_interrupts/lapic.h"
#include "../qkr_interrupts/processors.h"
void kernel_main(KernelGlobalData * kgd) {
	// This is the main kernel function.
	// The function should not return.
	screen_clear();
	screen_write_string("Hello! This is the kernel talking to you!", TRUE);
	screen_set_color(B_GREEN, MAGNETA);
	screen_write_string("COOL! The loader works, the screen works, and the bootloader done successfully!", TRUE);
	screen_set_color(B_CYAN, BLACK);
	screen_write_string("We actually can start to initialize the kernel!", TRUE);
	//char * a = 0xffffffffffffffff;
	//*a= 1;
	//while (TRUE);
}
uint64 count;
void my_cb() {
	count++;
	screen_printf("timer jumped! %d", count, 0, 0, 0);
	if (count == 10) lapic_timer_start(1000 * 1000 * 10 , FALSE);
}
QResult qkr_main(KernelGlobalData * kgd) {
	count = 0;
	kernel_main(kgd);
	void* x = NULL;
	void* y = NULL;
	void* z = NULL;
	lapic_timer_set_callback_function(my_cb);
	lapic_timer_start(1000*1000 * 2, TRUE);
	screen_write_string("asdfa",TRUE);
	while (1) {
		if (g_lapic_regs->current_count > 0) {
			//screen_printf("cur: %x, initial: %x", g_lapic_regs->current_count, g_lapic_regs->initial_count, 0, 0);
			//lapic_timer_start(1000 * 1000, -1);
			//screen_new_line();
		}
	}
	x = kheap_alloc(0x253);
	y = kheap_alloc(0x1652);
	z = kheap_alloc(0x100253);

	kheap_free(y);
	y = kheap_alloc(0x1652);

	kheap_free(x);
	kheap_free(z);
	kheap_free(y);
	y = kheap_alloc(0x1652);
	x = kheap_alloc(0x253);
	z = kheap_alloc(0x100253);
	
	kheap_free(z);
	kheap_free(y);
	kheap_free(x);
	x = kheap_alloc(0x1000);
	QHandle file = create_qbject("FS/FS0/BOOT.TXT",ACCESS_READ);
	uint64 bytes_read;
	read_qbject(file, x, 0, 0x200, &bytes_read);
	return QSuccess;
}