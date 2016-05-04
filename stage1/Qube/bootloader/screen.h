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
void _printf(ScreenHandle * scr, char * fmt, uint64 param1, uint64 param2, uint64 param3, uint64 param4);

// Use these macros:
#define INIT_SCREEN(/* ScreenHandle * */ scr, /* void* */ first_MB_ptr) {_init_screen(scr, first_MB_ptr);}
#define PUTS(/* ScreenHandle* */ scr, /* char* */ str) {char __temp_char_arr__[] = {str};_puts(scr, __temp_char_arr__);}
#define WRITE(/* ScreenHandle* */ scr, /* char* */ str) {_write(scr, str);}
#define NEWLINE(/* ScreenHandle* */ scr) {_newline(scr);}

#define PRINTF4(scr, fmt, param1, param2, param3, param4) {char __temp_char_arr__[] = {fmt};_printf(scr, __temp_char_arr__, (uint64)param1, (uint64)param2, (uint64)param3, (uint64)param4);}
#define PRINTF3(scr, fmt, param1, param2, param3) PRINTF4(scr, fmt, param1, param2, param3, 0);
#define PRINTF2(scr, fmt, param1, param2) PRINTF3(scr, fmt, param1, param2, 0);
#define PRINTF1(scr, fmt, param1) PRINTF2(scr, fmt, param1, 0);
#define PRINTF(scr, fmt) PRINTF1(scr, fmt, 0);





#define DBG_PRINT(str) PUTS(screen_ptr, str);
#define DBG_PRINTF(str) PRINTF(screen_ptr, str);
#define DBG_PRINTF1(str, p1) PRINTF1(screen_ptr, str, p1);
#define DBG_PRINTF2(str, p1, p2) PRINTF2(screen_ptr, str, p1, p2);
#define DBG_PRINTF3(str, p1, p2, p3) PRINTF3(screen_ptr, str, p1, p2, p3);
#define DBG_PRINTF4(str, p1, p2, p3, p4) PRINTF4(screen_ptr, str, p1, p2, p3, p4);
#define ENTER NEWLINE(screen_ptr);
#define DBG_WRITE(str) WRITE(screen_ptr, str);
#define DBG_PUTS(str) PUTS(screen_ptr, str);