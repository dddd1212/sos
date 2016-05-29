#include "../Common/Qube.h"
#include "screen.h"
#include "../libc/string.h"

Screen g_screen;
QResult qkr_main(KernelGlobalData * kgd) {
	return screen_init(kgd->first_MB);
}

QResult screen_init(void * first_MB_ptr) {
	g_screen.color = DEFAULT_COLOR;
	g_screen.start_screen_ptr = (short *)((char*)first_MB_ptr + SCREEN_ADDR_OFFSET);
	g_screen.cur_screen_ptr = g_screen.start_screen_ptr;
	g_screen.end_screen_ptr = g_screen.start_screen_ptr + COLS*ROWS;
	return QSuccess;
}
QResult screen_set_color(Color foreground, Color background)
{
	g_screen.color = foreground | (background << 4);
	return QSuccess;
}

QResult screen_write_string(char * str, BOOL newline)
{
	return screen_write_buffer(str, strlen(str), newline);
}

void _scroll_if_need() {
	while (g_screen.cur_screen_ptr >= g_screen.end_screen_ptr) { // Screen overflow! scroll one raw down.
		memcpy((char *)g_screen.start_screen_ptr, (char *)(g_screen.start_screen_ptr + COLS), COLS * (ROWS - 1) * sizeof(*g_screen.start_screen_ptr));
		memset((char *)(g_screen.start_screen_ptr - COLS), 0, COLS * sizeof(*g_screen.start_screen_ptr)); // delete the last row.
		g_screen.cur_screen_ptr -= COLS; // One raw up.
	}
	return;
}

QResult screen_write_buffer(char * buf, int size, BOOL newline)
{
	Screen * screen = &g_screen;
	for (int i = 0; i < size ; i++) {
		_scroll_if_need();
		if (*buf == '\r') {
			g_screen.cur_screen_ptr -= ((g_screen.cur_screen_ptr - g_screen.start_screen_ptr) % COLS);
		} else if (*buf == '\n') {
			screen_new_line();
		} else if (*buf == '\b') {
			if ((g_screen.cur_screen_ptr - g_screen.start_screen_ptr) % COLS) g_screen.cur_screen_ptr--;
		} else if (*buf == '\t') {
			g_screen.cur_screen_ptr += (4 - (g_screen.cur_screen_ptr - g_screen.start_screen_ptr)) % TAB_SIZE;
			_scroll_if_need();
		} else {
			*g_screen.cur_screen_ptr = (g_screen.color<<8) | *buf;
			g_screen.cur_screen_ptr++;
		}
		buf++;
		
	}
	if (newline) return screen_new_line();
	return QSuccess;
}


QResult screen_new_line()
{
	g_screen.cur_screen_ptr += COLS - ((g_screen.cur_screen_ptr - g_screen.start_screen_ptr) % COLS);
	_scroll_if_need();
	return QSuccess;
}

QResult screen_clear()
{
	memset((void *)g_screen.start_screen_ptr, '\x00', ROWS*COLS*sizeof(*g_screen.start_screen_ptr));
	g_screen.cur_screen_ptr = g_screen.start_screen_ptr;	
	return QSuccess;
}

QResult screen_locate(int x, int y)
{
	if (x > ROWS || y > COLS) return QFail;
	g_screen.cur_screen_ptr = g_screen.start_screen_ptr + COLS * x + y;
	return QSuccess; 
}

// TODO: This is temporary. we need to remove this function.
void screen_printf(char * fmt, uint64 param1, uint64 param2, uint64 param3, uint64 param4) {
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
			}
			else if (*fmt == 's') {
				screen_write_string((char *)params[param_idx], FALSE);
				param_idx++;
				fmt++;
				continue;
			}
			else {
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
				screen_write_string(&num_arr[num_arr_idx + 1], FALSE);
				fmt++;
				continue;
			}
		}
		// regular
		one_char[0] = *fmt;
		screen_write_string(&one_char[0], FALSE);
		fmt++;
	}
	return;
}