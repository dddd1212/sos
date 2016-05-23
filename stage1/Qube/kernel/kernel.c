#include "kernel.h"
#include "../screen/screen.h"
#include "../Common/Qube.h"

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
}


