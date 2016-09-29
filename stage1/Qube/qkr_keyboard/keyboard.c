#include "../Common/Qube.h"
#include "../qkr_interrupts/interrupts.h"
#include "../screen/screen.h"
#include "../qkr_interrupts/ioapic.h"
#include "../libc/qqueue.h"
#include "keyboard.h"
#include "../libc/string.h"
QQueue * g_scancodes_queue;
QQueue * g_ascii_queue;
typedef struct {
	BOOL rshift;
	BOOL lshift;
	
	BOOL rctrl;
	BOOL lctrl;
	
	BOOL ralt;
	BOOL lalt;

} KeyboardState;

typedef enum {
	SCAN_SM_REGULAR = 0,
	SCAN_SM_SECOND = 1,
} ScanStateMachine;

ScanStateMachine g_state;
KeyboardState g_keyboard_state;
void keyboard_handler(ProcessorContext * regs) {
	uint8 byte = __in8(0x60);
	int count = 0;
	while (1) {
		count++;
		if (count % 0x5000000 == 0) {
			//screen_printf("timer! %x\n", count, 0, 0, 0);
			return;
		}
	}
	screen_printf("Key pressed: %x\n", byte,0,0,0);
	// Now we want to handle the key. When threads will be here, we should do the most of the work in DPC-thread.

}
uint8 getch() {
	uint8 ret;
	while (!deqqueue(g_ascii_queue, &ret));
	return ret;
}

void empty_scancodes_queue() {
	uint8 scancode;
	BOOL ret;
	BOOL stop;
	while (qqueue_cur_size(g_scancodes_queue) > 0) {
		stop = FALSE;
		ret = deqqueue(g_scancodes_queue, &scancode);
		if (ret == FALSE) return;
		uint8 my_code;
		if (scancode == SCAN_CODE_1_SECOND) {
			g_state = SCAN_SM_SECOND;
			continue;
		}
		if (scancode >= 0x80) {
			scancode -= 0x80;
			stop = TRUE;
		}
		if (g_state == SCAN_SM_REGULAR) {
			my_code = SCAN_CODE_1_TABLE[scancode];
		} else {
			my_code = SCAN_CODE_1_SECONDARY_TABLE[scancode];
			g_state = SCAN_SM_REGULAR;
		}
		switch (my_code) {
		case L_SHFT:
			g_keyboard_state.lshift = !stop;
			break;
		case R_SHFT:
			g_keyboard_state.rshift = !stop;
			break;
		case L_CTRL:
			g_keyboard_state.lctrl = !stop;
			break;
		case R_CTRL:
			g_keyboard_state.rctrl = !stop;
			break;
		case L_ALT:
			g_keyboard_state.lalt = !stop;
			break;
		case R_ALT:
			g_keyboard_state.ralt = !stop;
			break;
		}
		
		if (my_code < 0xc0 && my_code > 0x80) { // keypad
			my_code = my_code & 0x7f; // remove the keypad bit.
		}
		if (g_keyboard_state.lshift || g_keyboard_state.rshift) {
			my_code = SHIFT_TABLE[my_code];
		}
		if (my_code >= 0x20 && my_code < 0x80) { // ascii.
			enqqueue(g_ascii_queue, &my_code);
			continue;
		}
	}
}


QResult qkr_main(KernelGlobalData *kgd) {
	g_scancodes_queue = create_qqueue(0x100, sizeof(uint8));
	g_ascii_queue = create_qqueue(0x100, sizeof(uint8));
	g_state = SCAN_SM_REGULAR;
	memset(&g_keyboard_state,0,sizeof(g_keyboard_state));
	/*
	uint8 byte;
// set scan code:
	while (__in8(0x64) & 2);
	__out8(0x64, 0xf0); // scan-code command
	
	while (__in8(0x64) & 2);
	__out8(0x60, 0x2); // set to 2.

// Wait for ack:
	while (!(__in8(0x64) & 1));
	byte = __in8(0x60);
	//if (byte != 0xFA) return QFail;
	
// Get the current scan-code:
	while (__in8(0x64) & 2);
	__out8(0x64, 0xf0); // scan-code command
	
	while (__in8(0x64) & 2);
	__out8(0x60, 0x0); // get

	while (!(__in8(0x64) & 1));
	byte = __in8(0x60);
	if (byte != 0xFA) return QFail;
	
	while (!(__in8(0x64) & 1));
	byte = __in8(0x60);
	
	screen_printf("Scan code is: %d\n", byte, 0, 0, 0);
	*/

	QResult ret = register_isr(APIC_KEYBOARD_CONTROLLER, keyboard_handler);
	if (ret == QFail) {
		screen_write_string("ERROR1!", TRUE);
		return QFail;
	}
	ret = configure_isa_interrupt(ISA_KEYBOARD_CONTROLLER, APIC_KEYBOARD_CONTROLLER, DM_FIXED, 0);
	if (ret == QFail) {
		screen_write_string("ERROR2!", TRUE);
		return QFail;
	}
	return QSuccess;
}