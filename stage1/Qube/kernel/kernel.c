#include "kernel.h"
#include "../screen/screen.h"
#include "../Common/Qube.h"

void kernel_main(KernelGlobalData * kgd) {
	// This is the main kernel function.
	// The function should not return.

	screen_write_string("Hello! This is the kernel talking to you!", TRUE);
	screen_set_color(B_GREEN, B_MAGNETA);
	screen_write_string("COOL! The loader works, the screen works, and the bootloader done successfully!", TRUE);
	screen_set_color(B_RED, B_BLACK);
	screen_write_string("We actually can start to initialize the kernel!", TRUE);

	while (TRUE);
}


QResult qkr_main(KernelGlobalData * kgd) {
	kernel_main(kgd);
}


