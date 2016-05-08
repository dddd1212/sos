#include "exports.h"
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
