#ifndef SYNC_H
#define SYNC_H
#include "../Common/Qube.h"
#include "../qkr_qbject_manager/Qbject.h"
#include "../qkr_interrupts/interrupts.h"
#define interlocked_cmpxchg __lockcmpxchg
#define interlocked_xchg __lockxchg
typedef void* Event;
EXPORT QResult create_event(Event* p_event);
EXPORT QResult delete_event(Event p_event);
EXPORT QResult set_event(Event a_event);
EXPORT QResult unset_event(Event a_event);
EXPORT QResult wait_for_event(Event a_event);
#endif
