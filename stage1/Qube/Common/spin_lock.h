#ifndef __SPIN_LOCK_H__
#define __SPIN_LOCK_H__

#include "intrinsics.h"
typedef int volatile * SpinLock;

void inline spin_init(SpinLock p) {
	*p = 0;
}

static inline void spin_lock(SpinLock p)
{
	while (!__qube_sync_bool_compare_and_swap(p, 0, 1))
	{
		while (*p) __qube_mm_pause();
	}
}

static inline void spin_unlock(SpinLock p)
{
	__qube_memory_barrier();
	*p = 0;
}

#endif // __SPIN_LOCK_H__
