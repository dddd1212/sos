#ifndef __SCREEN_H__
#define __SCREEN_H__

#include "../Common/Qube.h"

typedef enum {
	BLACK = 0,
	BLUE = 1,
	RED = 2,
	MAGNETA = 3,
	GREEN = 4,
	CYAN = 5,
	YELLOW = 6,
	WHITE = 7,
	B_BLACK = 8,
	B_BLUE = 9,
	B_RED = 10,
	B_MAGNETA = 11,
	B_GREEN = 12,
	B_CYAN = 13,
	B_YELLOW = 14,
	B_WHITE = 15,
} Color;
#define COLS 80
#define ROWS 25
#define SCREEN_ADDR_OFFSET 0xB8000

#define DEFAULT_COLOR 0x07 // White on black.
#define TAB_SIZE 4
typedef struct {
	short * start_screen_ptr;
	short * cur_screen_ptr;
	short * end_screen_ptr;
	char color;
} Screen;


EXPORT QResult screen_set_color(Color foreground, Color background);
EXPORT QResult screen_write_string(char * str, BOOL newline);
EXPORT QResult screen_write_buffer(char * buf, int size, BOOL newline);
EXPORT QResult screen_new_line();
EXPORT QResult screen_clear();
EXPORT QResult screen_locate(int x, int y);
EXPORT void screen_printf(char * fmt, uint64 param1, uint64 param2, uint64 param3, uint64 param4);
EXPORT QResult screen_init(void * first_MB_ptr);
EXPORT QResult qkr_main(KernelGlobalData * kgd);




#endif // __SCREEN_H__