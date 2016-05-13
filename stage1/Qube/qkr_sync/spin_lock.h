#ifndef __SPIN_LOCK_H__
#define __SPIN_LOCK_H__
#include "../Common/intrinsics.h"

void inline spin_init(int *p) {
	*p = 0;
}

void inline spin_lock(int volatile *p)
{
	while (!__qube_sync_bool_compare_and_swap(p, 0, 1))
	{
		while (*p) __qube_mm_pause();
	}
}

void inline spin_unlock(int volatile *p)
{
	__qube_memory_barrier();
	*p = 0;
}
#endif // __SPIN_LOCK_H__
