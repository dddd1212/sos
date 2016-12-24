#include "qnet_threads_pool.h"
#include "qnet_os.h"

QNetThreadsPool * qnet_tpool_create_pool() {
	QNetThreadsPool * pool = qnet_alloc(sizeof(QNetThreadsPool));
	if (pool == NULL) goto fail0;

	if (pool->stop_event = qnet_create_event(FALSE /* not auto reset.*/) != QSuccess) {
		goto fail1;
	}
	qnet_set_event(pool->stop_event);

	if (pool->threads_zero_event = qnet_create_event(FALSE /* not auto reset.*/) != QSuccess) {
		goto fail2;
	}
	qnet_set_event(pool->threads_zero_event);

	if (pool->start_thread_mutex = qnet_create_mutex(TRUE /* Fast mutex */) != QSuccess) {
		goto fail3;
	}
	pool->threads_count = 0;
	return QSuccess;
fail4:
	qnet_delete_mutex(pool->start_thread_mutex);
fail3:
	qnet_delete_event(pool->threads_zero_event);
fail2:
	qnet_delete_event(pool->stop_event);
fail1:
	qnet_free(pool);
fail0:
	return NULL;
}
QResult qnet_tpool_delete_pool(QNetThreadsPool * pool) {
	if (qnet_wait_for_event(pool->stop_event, 0) == FALSE) return QFail;
	if (qnet_wait_for_event(pool->threads_zero_event, STOP_POOL_TIMEOUT) == FALSE) return QFail;
	qnet_delete_mutex(pool->start_thread_mutex);
	qnet_delete_event(pool->threads_zero_event);
	qnet_delete_event(pool->stop_event);
	qnet_free(pool);
	return QSuccess;
}
// This routine init the pool struct and start it.
QResult qnet_tpool_start_pool(QNetThreadsPool * pool) {
	if (pool->threads_count != 0) return QFail;
	if (qnet_wait_for_event(pool->stop_event, 0) != TRUE) return QFail;
	qnet_reset_event(pool->stop_event);
	return QSuccess;
}

QResult qnet_tpool_start_new(QNetThreadsPool * pool, QNetThreadFunc * thread_func, void * param) {
	QResult ret;
	// First check if we have permission to start new thread:
	qnet_acquire_mutex(pool->start_thread_mutex);
	if (qnet_wait_for_event(pool->stop_event, 0) == TRUE) { // The event is set!
		qnet_release_mutex(pool->start_thread_mutex);
		return QFail;
	}
	qnet_reset_event(pool->threads_zero_event);
	pool->threads_count++;
	qnet_release_mutex(pool->start_thread_mutex);
	// From here, from the point of view of the pool manager, the thread exists, and from now on, if the pool wants to be closed,
	// the manager will wait until we decrease the threads_count.
	if (qnet_start_thread(thread_func, param) == QSuccess) {
		return QSuccess; // The thread needs to call qnet_tpool_thread_finish when it finish.
	}
	qnet_tpool_thread_finish(pool);
	return QFail;
}

QResult qnet_tpool_thread_finish(QNetThreadsPool * pool) {
	qnet_acquire_mutex(pool->start_thread_mutex);
	pool->threads_count--;
	if (pool->threads_count == 0) {
		qnet_set_event(pool->threads_zero_event);
	}
	qnet_release_mutex(pool->start_thread_mutex);
	return QSuccess;
}

QResult qnet_tpool_stop_pool(QNetThreadsPool * pool) {
	// We have to do the destroy thing while no thread is in start process:
	qnet_acquire_mutex(pool->start_thread_mutex);
	qnet_set_event(pool->stop_event);
	qnet_release_mutex(pool->start_thread_mutex); // after the event is set, no one will increase the pool->threads_count.
	if (qnet_wait_for_event(pool->threads_zero_event, STOP_POOL_TIMEOUT) == FALSE) {
		// timeout!
		return QFail;
	}
	// If we here, that means that the threads_zero_event is set, and the stop_event is set, and the threads_count is 0.
	return QSuccess;
}