#ifndef SYNC_H
#define SYNC_H
#include "../Common/Qube.h"
#include "../QbjectManager/Qbject.h"
typedef void* Event;
EXPORT QResult create_event(Event* p_event);
EXPORT QResult delete_event(Event p_event);
EXPORT QResult set_event(Event a_event);
EXPORT QResult unset_event(Event a_event);
EXPORT QResult wait_for_event(Event a_event);
#endif
