#ifndef __SPIN_LOCK_H__
#define __SPIN_LOCK_H__

#include "intrinsics.h"

static void inline spin_init(SpinLock * p)  __attribute__((always_inline));
static void inline spin_lock(SpinLock * p)  __attribute__((always_inline));
static void inline spin_unlock(SpinLock * p)  __attribute__((always_inline));

static void inline spin_init(SpinLock * p)  {
	*p = 0;
}
 
static void inline spin_lock(SpinLock * p)  {
	while (!__qube_sync_bool_compare_and_swap(p, 0, 1))
	{
		while (*p) __qube_mm_pause();
	}
}

static void inline spin_unlock(SpinLock * p) {
	__qube_memory_barrier();
	*p = 0;
}

#endif // __SPIN_LOCK_H__
