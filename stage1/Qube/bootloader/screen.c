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
}

// Don't call this function implicit:
void _puts(ScreenHandle * scr, char * str) {
	_write(scr, str);
	_newline(scr);
}