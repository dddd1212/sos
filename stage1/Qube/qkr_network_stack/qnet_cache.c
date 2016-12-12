#include "qnetstack.h"
#include "qnet_os.h"
struct _QNetCacheEntryCommon {
	struct _QNetCacheEntryCommon * next;
	struct _QNetCacheEntryCommon * prev;
	uint64 timeout; // in seconds
							// Here you should put the entry itself.
							// You need to create a struct with the CacheEntryCommon in it start and call the functions with cast.
};
void qnet_cache_copy_common_data(QNetCache * qcache, QNetCacheEntryCommon * dst, QNetCacheEntryCommon * src) {
	dst->next = NULL;
	dst->prev = NULL;
	dst->timeout = src->timeout;
	return;
}
typedef struct _QNetCacheEntryCommon QNetCacheEntryCommon;

typedef BOOL(*QNetCacheSearchFunc)(QNetCache *qcache, QNetCacheEntryCommon * entry, void * user_defined_struct);
typedef uint32(*QNetCacheCompareFunc)(QNetCache *qcache, QNetCacheEntryCommon * first, QNetCacheEntryCommon * second);
typedef BOOL (*QNetCacheCopyFunc)(QNetCache *qcache, QNetCacheEntryCommon * dst, QNetCacheEntryCommon * src); // copy all the user struct.
typedef BOOL(*QNetCacheFreeEntryFunc)(QNetCache *qcache, QNetCacheEntryCommon * dst); // free the user data of the entry.
struct _QNetCache {
	uint32 max_size;
	uint32 actual_size;
	uint64 last_check_time;
	QNetCacheSearchFunc search_func;
	QNetCacheCompareFunc compare_func;
	QNetCacheCopyFunc copy_func;
	QNetCacheFreeEntryFunc free_func;
	struct _QNetCacheEntryCommon * first;
	struct _QNetCacheEntryCommon * last;
	QNetMutex * cache_mutex;
};
typedef struct _QNetCache QNetCache;
// creation:
QResult qnet_cache_create(QNetCache * qcache, uint32 real_entry_size, QNetCacheSearchFunc search_func, 
																	  QNetCacheCompareFunc compare_func, 
																	  QNetCacheCopyFunc copy_func, 
																	  QNetCacheFreeEntryFunc free_func,
					      uint32 max_size) {
	qcache = (QNetCache *)qnet_alloc(sizeof(QNetCache));
	qcache->max_size = max_size;
	qcache->actual_size = 0;
	qcache->search_func = search_func;
	qcache->compare_func = compare_func;
	qcache->copy_func = copy_func;
	qcache->free_func = free_func;
	qcache->first = NULL;
	qcache->last = NULL;
	qcache->cache_mutex = qnet_create_mutex(TRUE); // fast mutex.
	qcache->last_check_time = qnet_get_cur_time();
	qcache->real_entry_size = real_entry_size;
}
QResult qnet_cache_destroy(QNetCache * qcache) {
	qnet_acquire_mutex(qcache->cache_mutex);
	qnet_cache_entries_free(qcache, qcache->first);
	qnet_free((void*)qcache);
	qnet_delete_mutex(qcache->cache_mutex);
	return QSuccess;
}
QNetCacheEntryCommon * qnet_cache_create_entry(QNetCache * qcache) {
	return (QNetCacheEntryCommon * )qnet_alloc(qcache->real_entry_size);
}
// insertion:
QResult qnet_cache_add_entry(QNetCache * qcache, QNetCacheEntryCommon * entry, uint32 timeout) {
	uint64 now = qnet_get_cur_time();
	entry->timeout = now + timeout;
	qnet_acquire_mutex(qcache->cache_mutex);
	QNetCacheEntryCommon * ret = qnet_cache_iterate(qcache, qnet_cache_check_ident, (void*)entry);
	if (ret != NULL) { // Out entry already in the cache!
		ret->timeout = now + timeout;
	}
	// If we need to add the entry to the cache:
	/* We don't need to do this beacuse we already iterate over the cache just before.
	if (qcache->max_size == qcache->actual_size) {
		qnet_cache_remove_invalids(qcache);
	}
	*/
	if (qcache->max_size == qcache->actual_size) {
		// remove the last one:
		qnet_cache_remove_entry(qcache, qcache->last);
	}

	if (qcache->first == NULL) {
		qcache->first = entry;
		qcache->last = entry;
		entry->next = NULL;
		entry->prev = NULL;
	} else {
		entry->next = qcache->first;
		entry->prev = NULL;
		qcache->first = entry;
	}
	qcache->actual_size += 1;
	qnet_delete_mutex(qcache->cache_mutex);
}
QResult qnet_cache_remove_entry(QNetCache * qcache, QNetCacheEntryCommon * entry) {
	qnet_acquire_mutex(qcache->cache_mutex);
	qcache->last = qcache->last->prev;
	qnet_cache_entries_free(qcache, qcache->last->next); // free the last one.
	qcache->last->next = NULL;
	qnet_release_mutex(qcache->cache_mutex);
}

QNetCacheEntryCommon * qnet_cache__iter_func__remove_invalids(QNetCache * qcache, QNetCacheEntryCommon * entry, void * param) {
	return NULL; // We just want to go over every entry and reomve the invalids.
}

QNetCacheEntryCommon * qnet_cache__iter_func__copy(QNetCache * qcache, QNetCacheEntryCommon * entry, void * param) {
	QNetCacheEntryCommon ** entries_ptr = (QNetCacheEntryCommon **) param;
	QNetCacheEntryCommon  * temp = qnet_cache_create_entry(qcache);
	if (temp == NULL) { // not enough memory.
		qnet_cache_entries_free(qcache, *entries_ptr);
		*entries_ptr = NULL;
		return (QNetCacheEntryCommon*)1; // error. stop iterate.
	}
	qnet_cache_copy_common_data(qcache, temp, entry);
	qcache->copy_func(qcache, temp, entry);
	if (*entries_ptr == NULL) {
		*entries_ptr = temp;
	} else {
		(*entries_ptr)->next = temp;
		temp->prev = (*entries_ptr);
	}
	return NULL; // continue iterate.
}

QNetCacheEntryCommon * qnet_cache__iter_func__find(QNetCache * qcache, QNetCacheEntryCommon * entry, void * param) {
	if (qcache->search_func(qcache, entry, param) == TRUE) return entry;
	return NULL;
}
QNetCacheEntryCommon * qnet_cache__iter_func__find_and_remove(QNetCache * qcache, QNetCacheEntryCommon * entry, void * param) {
	if (qcache->search_func(qcache, entry, param) == TRUE) {
		qnet_cache_remove_entry(qcache, entry);
		return (QNetCacheEntryCommon *) 1;
	}
	return NULL;
}
QNetCacheEntryCommon * qnet_cache__iter_func__find_and_remove_all(QNetCache * qcache, QNetCacheEntryCommon * entry, void * param) {
	if (qcache->search_func(qcache, entry, param) == TRUE) {
		qnet_cache_remove_entry(qcache, entry);
	}
	return NULL;
}

QNetCacheEntryCommon * qnet_cache__iter_func__check_ident(QNetCache * qcache, QNetCacheEntryCommon * entry, void * param) {
	QNetCacheEntryCommon * new_entry = (QNetCacheEntryCommon *)param;
	if (qcache->compare_func(qcache, entry, param) == 0) return entry;
	return NULL;
}
typedef QNetCacheEntryCommon * (*QNetCacheIterateFunc)(QNetCache * qcache, QNetCacheEntryCommon * entry, void * param);
QNetCacheEntryCommon * qnet_cache_iterate(QNetCache * qcache, QNetCacheIterateFunc func, void * param) {
	QNetCacheEntryCommon * entry;
	QNetCacheEntryCommon * next;
	QNetCacheEntryCommon * ret = NULL;
	qnet_acquire_mutex(qcache->cache_mutex);

	uint64 now = qnet_get_cur_time();
	qcache->last_check_time = now;
	for (entry = qcache->first; entry != NULL; entry = next) {
		next = entry->next;
		if (now > entry->timeout) {
			qnet_cache_remove_entry(qcache, entry);
		}
		ret = func(qcache, entry, param);
		if (ret != NULL) {
			break;
		}
		

	}
	qnet_release_mutex(qcache->cache_mutex);
	return ret;
}

QResult qnet_cache_remove_invalids(QNetCache * qcache) {
	qnet_acquire_mutex(qcache->cache_mutex);
	qnet_cache_iterate(qcache, qnet_cache__iter_func__remove_invalids, NULL);
	qnet_release_mutex(qcache->cache_mutex);
	return QSuccess;
}

void qnet_cache_remove_entry(QNetCache * qcache, QNetCacheEntryCommon * entry) {
	qnet_acquire_mutex(qcache->cache_mutex);
	if (entry == qcache->first) {
		qcache->first = entry->next;
	} else {
		entry->prev->next = entry->next;
	}
	if (entry == qcache->last) {
		qcache->last = entry->prev;
	} else {
		entry->next->prev = entry->prev;
	}
	entry->next = NULL;
	entry->prev = NULL;
	qnet_cache_entries_free(qcache, entry);
	qnet_release_mutex(qcache->cache_mutex);
	return;
}

// iteration and search:
QNetCacheEntryCommon * qnet_cache_get_copy(QNetCache * qcache) {
	QNetCacheEntryCommon * ret = NULL;
	uint32 ret_val = (uint32) qnet_cache_iterate(qcache, qnet_cache__iter_func__copy, (void*)(&ret));
	if (ret_val == 1) { // error!
		return NULL;
	}
	return ret; // The entries saved in this variable.
}

BOOL qnet_cache_find(QNetCache * qcache, void * user_define_struct) {
	return qnet_cache_iterate(qcache, qnet_cache__iter_func__find, user_define_struct);
}
BOOL qnet_cache_find_and_remove(QNetCache * qcache, void * user_define_struct) {
	return qnet_cache_iterate(qcache, qnet_cache__iter_func__find_and_remove, user_define_struct);
}
void qnet_cache_find_and_remove_all(QNetCache * qcache, void * user_define_struct) {
	return qnet_cache_iterate(qcache, qnet_cache__iter_func__find_and_remove_all, user_define_struct);
}
QResult qnet_cache_entries_free(QNetCache qcache, QNetCacheEntryCommon * entries) {
	QNetCacheEntryCommon * entry;
	QNetCacheEntryCommon * next = NULL;
	for (entry = entries; entry; entry = next) {
		next = entry->next;
		qcache->free_func(qcache, entry);
		qnet_free(entry);
	}
	return QSuccess;
}