#ifndef __QNET_OS__
#define __QNET_OS__
// We try to put here whole the os specific things to make the network stack portable.
// We only use the primitive defines of Qube.
// If you want to use this stack, just implement all of these functions.
#include "../Common/Qube.h"

#include "qnet_defs.h"

// os specific interface struct. This struct is member of QnetInterface.
struct QNetOsInterface;

struct _QNetMutex;
typedef struct _QNetMutex QNetMutex;
struct _QNetEvent;
typedef struct _QNetEvent QNetEvent;

#define PANIC

// function to start a new thread.
QResult qnet_start_thread(QNetThreadFunc * thread_func, void * param);

// function that allocate buffer to save packets data.
uint8 * qnet_alloc_packet(uint32 size);

// Function that free data that allocated with the qnet_alloc_packet or given to QNetStack by the recv_frame_func function.
void qnet_free_packet(uint8 * addr);

// General Functions for allocate and free memory (not for packets data).
uint8 * qnet_malloc(uint32 size);
uint8 * qnet_realloc(uint8 * ptr, uint32 size);
void qnet_free(uint8 * addr);

// endian swapping. In big endian machines these functions does nothing.
uint16 qnet_swap16(uint16);
uint32 qnet_swap32(uint32);

uint8 * qnet_memcpy(uint8 * dst, uint8 * src, uint32 size);
uint8 * qnet_memset(uint8 * dst, char c, uint32 size);
uint32 qnet_memcmp(uint8 * first, uint8 * second, uint32 size);
// return NULL in failure
QNetMutex * qnet_create_mutex(BOOL is_fast);

// panic on failure
void qnet_acquire_mutex(QNetMutex * mutex);

// panic on failure
void qnet_release_mutex(QNetMutex * mutex);

// panic on failure
void qnet_delete_mutex(QNetMutex * mutex);

// events api:
// The implementation may use the *event_id address for internal use.

// Create new event object. return TRUE on success.
QNetEvent * qnet_create_event(BOOL is_auto_reset);
void qnet_delete_event(QNetEvent *ev);

// Wait until event_id will be in signal state.
// Return TRUE if signaled, return FALSE if timeout.
BOOL qnet_wait_for_event(QNetEvent *event_id, uint32 milliseconds);

// signal and reset the event.
void qnet_set_event(QNetEvent *event_id);
void qnet_reset_event(QNetEvent *event_id);



uint64 qnet_get_cur_time();
#endif // __QNET_OS__