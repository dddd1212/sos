#ifndef __QNET_OS__
#define __QNET_OS__
// We try to put here whole the os specific things to make the network stack portable.
// We only use the primitive defines of Qube.
// If you want to use this stack, just implement all of these functions.
#include "../Common/Qube.h"

#include "qnet_defs.h"

// os specific interface struct. This struct is member of QnetInterface.
struct QNetOsInterface;

struct _QNetMutex; // mutex that could be held for long time.
struct _QNetFastMutex; // mutex that held for short time.
typedef struct _QNetMutex QNetMutex;
typedef struct _QNetFastMutex QNetFastMutex;


// function to start a new thread.
QResult qnet_start_thread(QnetThreadFunc * thread_func, void * param);

// function that allocate buffer to save packets data.
uint8 * qnet_alloc_packet(uint32 size);

// Function that free data that allocated with the qnet_alloc_packet or given to QNetStack by the recv_frame_func function.
void qnet_free_packet(uint8 * addr);

// General Functions for allocate and free memory (not for packets data).
uint8 * qnet_alloc(uint32);
void qnet_free(uint8 *);

// endian swapping. In big endian machines these functions does nothing.
uint16 qnet_swap16(uint16);
uint32 qnet_swap32(uint32);


// return NULL in failure
QNetMutex * qnet_create_mutex(BOOL is_fast);

// panic on failure
void qnet_acquire_mutex(QNetMutex * mutex);

// panic on failure
void qnet_release_mutex(QNetMutex * mutex);

// panic on failure
void qnet_delete_mutex(QNetMutex * mutex);

#endif // __QNET_OS__