#include "kernel.h"
#include "../screen/screen.h"
#include "../Common/Qube.h"
#include "../MemoryManager/memory_manager.h"
#include "../QbjectManager/Qbject.h"
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


QResult qkr_main(KernelGlobalData * kgd) {
	kernel_main(kgd);
	void* x = NULL;
	void* y = NULL;
	void* z = NULL;
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