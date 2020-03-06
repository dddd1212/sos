#ifndef __QNET_THREADS_POOL_H__
#define __QNET_THREADS_POOL_H__
#include "qnet_defs.h"

#define STOP_POOL_TIMEOUT 1
typedef struct {
	uint32 threads_count;
	QNetEvent * threads_zero_event;
	QNetEvent * stop_event;

	QNetMutex * start_thread_mutex;
	
} QNetThreadsPool;
QNetThreadsPool * qnet_tpool_create_pool();
QResult qnet_tpool_delete_pool(QNetThreadsPool *);

QResult qnet_tpool_start_new(QNetThreadsPool * pool, QNetThreadFunc * thread_func, void * param);
QResult qnet_tpool_thread_finish(QNetThreadsPool * pool);

// These functions are not thread safe!
QResult qnet_tpool_start_pool(QNetThreadsPool * pool);
QResult qnet_tpool_stop_pool(QNetThreadsPool * pool);



#endif // __QNET_THREADS_POOL_H__

