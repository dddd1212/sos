#ifndef __STDIO__H__
#define __STDIO__H__
#include "stdarg.h"
#include "../Common/Qube.h"
// The function type used by __sprintf. The function use ctx and can update its * to maintain char writing.
// In case of success, the function returns TRUE. in case of failure the function returns FALSE.
typedef BOOL(*WriteCharFunc)(void * ctx, int8 c);

uint32 vgprintf(WriteCharFunc f, void * ctx, int8 * fmt, va_list ap);

// Security note: This function is NOT secure. use it just if you know for sure that the buffer is big enough to holt the output!
int sprintf(int8 * out, int8 * fmt, ...);
#endif // __STDIO__H__
