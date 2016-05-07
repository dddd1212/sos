#include "screen.h"
#include "libc.h"

void _init_screen(ScreenHandle * scr, void * first_MB_ptr) {
	ASSERT(COLS % sizeof(uint64) == 0);
	// We assume that we already in VGA mode.
	scr->start_screen_ptr = (short *)((char*)first_MB_ptr + SCREEN_ADDR_OFFSET);
	scr->cur_screen_ptr = scr->start_screen_ptr;
	scr->end_screen_ptr = scr->start_screen_ptr + COLS*RAWS;
}

void _write(ScreenHandle * scr, char * str) {
	while (*str != '\x00') {
		if (scr->cur_screen_ptr >= scr->end_screen_ptr) { // Screen overflow! scroll one raw down.
			memcpy((char *) scr->start_screen_ptr, (char *) (scr->start_screen_ptr + COLS), COLS * (RAWS - 1) * sizeof(*scr->start_screen_ptr));
			memset((char *)scr->end_screen_ptr - COLS, 0, COLS); // delete the last row.
			scr->cur_screen_ptr -= COLS; // One raw up.
		}
		*scr->cur_screen_ptr = COLOR | *str;
		str++;
		scr->cur_screen_ptr++;
	}
	return;
}
void _newline(ScreenHandle * scr) {
	scr->cur_screen_ptr += COLS - ((scr->cur_screen_ptr - scr->start_screen_ptr) % COLS);
	if (scr->cur_screen_ptr >= scr->end_screen_ptr) { // Screen overflow! scroll one raw down.
		memcpy((char *)scr->start_screen_ptr, (char *)(scr->start_screen_ptr + COLS), COLS * (RAWS - 1) * sizeof(*scr->start_screen_ptr));
		memset((char *)(scr->end_screen_ptr - COLS), 0, COLS * sizeof(*scr->start_screen_ptr)); // delete the last row.
		scr->cur_screen_ptr -= COLS; // One raw up.
	}
}

// Don't call this function implicit:
void _puts(ScreenHandle * scr, char * str) {
	_write(scr, str);
	_newline(scr);
}

void _printf(ScreenHandle * scr, char * fmt, uint64 param1, uint64 param2, uint64 param3, uint64 param4) {
	uint64 params[4] = { param1, param2, param3, param4 };
	int param_idx = 0;
	uint64 param;
	char num_arr[21]; // max uint64 base10 size + null.
	int num_arr_idx = 0;
	int base = 0;
	char one_char[2];
	one_char[1] = '\x00';
	while (*fmt != '\x00') {
		if (*fmt == '%') {
			fmt++;
			if (*fmt == 'd') {
				base = 10;
			} 
			else if (*fmt == 'x') {
				base = 16;
			} else if (*fmt == 's') {
				_write(scr, (char *)params[param_idx]);
				param_idx++;
				fmt++;
				continue;
			} else {
				fmt--;
			}
			if (base) { // handle %x, %d:
				param = params[param_idx];
				param_idx++;
				num_arr_idx = 20; // start from the end
				num_arr[num_arr_idx] = '\x00';
				num_arr_idx--;
				do {
					int digit = param % base;
					if (digit < 10) {
						num_arr[num_arr_idx] = '0' + digit;
					}
					else {
						num_arr[num_arr_idx] = 'A' - 10 + digit;
					}
					num_arr_idx--;
					param /= base;
				} while (param != 0);
				_write(scr, &num_arr[num_arr_idx + 1]);
				fmt++;
				continue;
			}
		}
		// regular
		one_char[0] = *fmt;
		_write(scr, &one_char[0]);
		fmt++;
	}
	return;
}