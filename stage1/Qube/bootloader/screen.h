#include "../Common/Qube.h"

#define SCREEN_ADDR_OFFSET 0xB8000
#define COLS 80
#define RAWS 25
#define COLOR 0x0a00


// To use the screen, don't use these functions. use the following macros:
void _init_screen(ScreenHandle * scr, void * first_MB_ptr);
void _write(ScreenHandle * scr, char * str);
void _newline(ScreenHandle * scr);
void _puts(ScreenHandle * scr, char * str);

// Use these macros:
#define INIT_SCREEN(/* ScreenHandle * */ scr, /* void* */ first_MB_ptr) {_init_screen(scr, first_MB_ptr);}
#define PUTS(/* ScreenHandle* */ scr, /* char* */ str) {char __temp_char_arr__[] = {str};_puts(scr, __temp_char_arr__);}
#define WRITE(/* ScreenHandle* */ scr, /* char* */ str) {_write(scr, str);}
#define NEWLINE(/* ScreenHandle* */ scr) {_newline(scr);}

#define DBG_PRINT(str) PUTS(screen_ptr, str);