#ifndef __QNET_OS__
#define __QNET_OS__
// We try to put here whole the os specific things to make the network stack portable.
// We only use the primitive defines of Qube.
// If you want to use this stack, just implement all of these functions.
#include "../Common/Qube.h"

#include "qnet_defs.h"

// os specific interface struct. This struct is member of QnetInterface.
struct QNetOsInterface;

// function to start a new thread.
QResult qnet_start_thread(QnetThreadFunc * thread_func, void * param);

// function that allocate buffer to save packets data.
uint8 * qnet_alloc_packet(uint32 size);

// Function that free data that allocated with the qnet_alloc_packet or given to QNetStack by the recv_frame_func function.
void qnet_free_packet(uint8 * addr);

// General Functions for allocate and free memory (not for packets data).
uint8 * qnet_alloc(uint32);
void qnet_free(uint8 *);

#endif // __QNET_OS__