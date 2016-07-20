#include "kernel.h"
#include "../screen/screen.h"
#include "../Common/Qube.h"
#include "../MemoryManager/memory_manager.h"
#include "../QbjectManager/Qbject.h"
#include "../qkr_interrupts/lapic.h"
#include "../qkr_interrupts/processors.h"
#include "../qkr_interrupts/ioapic.h"
#include "../qkr_interrupts/interrupts.h"
#include "../qkr_acpi/acpi.h"
#include "../libc/string.h"
#include "../libc/stdio.h"
void kernel_main(KernelGlobalData * kgd) {
	// This is the main kernel function.
	// The function should not return.
	screen_clear();
	screen_write_string("Hello! This is the kernel talking to you!", TRUE);
	screen_set_color(B_GREEN, MAGNETA);
	screen_write_string("COOL! The loader works, the screen works, and the bootloader done successfully!", TRUE);
	screen_set_color(B_CYAN, BLACK);
	screen_write_string("We actually can start to initialize the kernel!", TRUE);
	int8 out[0x1000];
	sprintf(out, "hello! this is int: %08d. Str%%in%123g is: %08s!\n hex:%04x, %04X, %x, %X", 1234,"aaa", 0x10a,0x23b,0x10c,0x99d);
	screen_write_string(out, TRUE);


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
void my_test_handler(ProcessorContext * regs) {
	screen_write_string("Here!", TRUE);
}
QResult qkr_main(KernelGlobalData * kgd) {
	count = 0;
	kernel_main(kgd);
	__int(100);
	QResult ret = register_isr(APIC_KEYBOARD_CONTROLLER, my_test_handler);
	
	if (ret == QFail) {
		screen_write_string("ERROR1!", TRUE);
		return QFail;
	}
	ret = configure_isa_interrupt(ISA_KEYBOARD_CONTROLLER, APIC_KEYBOARD_CONTROLLER, DM_FIXED, 0);
	if (ret == QFail) {
		screen_write_string("ERROR2!", TRUE);
		return QFail;
	}
	lapic_timer_set_callback_function(my_cb);
	lapic_timer_start(1000 * 1000*10, TRUE);
	screen_write_string("asdfa", TRUE);
	while (1) {

	}
	enable_isa_interrupt(ISA_KEYBOARD_CONTROLLER);
	while (1) {

	}
	
	uint8* x = NULL;
	void* y = NULL;
	void* z = NULL;
	x = (uint8*)commit_pages(KHEAP, 16 * 0x1000 * 0x1000);
	assign_committed(x, 16 * 0x1000 * 0x1000, 0xf0000000);
	for (uint32 i = 0; i < (1 << 13); i++) {
		uint32 q = *(uint32*)(x + (1 << 12)*i);
		if ((uint16)q != 0xFFFF) {
			char k[0x48];
			memcpy(k, (x + (1 << 12)*i), 0x48);
 			screen_write_string("asdfa",k[0]!=0xFE);
		}
	}
	/*
	for (uint32 i = 0; i < 0x1000000; i++) {
		uint32 t = *(uint8*)((uint8*)x + i);
		if (t != 0xff) {
			*(uint8*)x = 9;
		}
	}
	*/
	/*
	for (uint32 i = 0; i < 0x1000 * 0x1000; i += 16) {
		if (*(uint64*)((uint8*)x + i) == 0x2052545020445352) {
			*(uint8*)x = 9;
		}
	}*/
	//unassign_committed(x, 0x1000 * 0x1000);
	free_pages(x);

	lapic_timer_set_callback_function(my_cb);
	lapic_timer_start(1000*1000 * 2, TRUE);
	screen_write_string("asdfa",TRUE);
	ACPITable* mcfg = get_acpi_table("MCFG");
	dump_table(mcfg);

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