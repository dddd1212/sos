#include "qnetstack.h"
#include "qnet_os.h"
#include "qnet_cache.h"


// creation:
QNetCache * qnet_cache_create(uint32 real_entry_size, QNetCacheEntrySearchFunc search_func, 
																	  QNetCacheEntryCompareFunc compare_func, 
																	  QNetCacheEntryCopyFunc copy_func, 
																	  QNetCacheEntryFreeFunc free_func,
					      uint32 max_size) {
	QNetCache * qcache = (QNetCache *)qnet_alloc(sizeof(QNetCache));
	if (qcache == NULL) return NULL;
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
	return qcache;
}
QResult qnet_cache_destroy(QNetCache * qcache) {
	qnet_acquire_mutex(qcache->cache_mutex);
	_qnet_cache_entries_free(qcache, qcache->first);
	qnet_free((void*)qcache);
	qnet_delete_mutex(qcache->cache_mutex);
	return QSuccess;
}
QNetCacheEntryCommon * qnet_cache_create_entry(QNetCache * qcache) {
	QNetCacheEntryCommon * ret = (QNetCacheEntryCommon * )qnet_alloc(qcache->real_entry_size);
	ret->next = NULL;
	ret->prev = NULL;
	ret->timeout = 0;
}

// insertion:
QResult qnet_cache_add_entry(QNetCache * qcache, QNetCacheEntryCommon * entry, uint32 timeout) {
	uint64 now = qnet_get_cur_time();
	entry->timeout = now + timeout;
	qnet_acquire_mutex(qcache->cache_mutex);
	QNetCacheEntryCommon * ret = _qnet_cache_iterate(qcache, _qnet_cache__iter_func__check_ident, (void*)entry);
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
		_qnet_cache_remove_entry(qcache, qcache->last);
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
void _qnet_cache_remove_entry(QNetCache * qcache, QNetCacheEntryCommon * entry) {
	if (entry == qcache->first) {
		qcache->first = entry->next;
	}
	else {
		entry->prev->next = entry->next;
	}
	if (entry == qcache->last) {
		qcache->last = entry->prev;
	}
	else {
		entry->next->prev = entry->prev;
	}
	entry->next = NULL;
	entry->prev = NULL;
	_qnet_cache_entries_free(qcache, entry);
	return;
}
QResult _qnet_cache_entries_free(QNetCache qcache, QNetCacheEntryCommon * entries) {
	QNetCacheEntryCommon * entry;
	QNetCacheEntryCommon * next = NULL;
	for (entry = entries; entry; entry = next) {
		next = entry->next;
		qcache->free_func(qcache, entry);
		qnet_free(entry);
	}
	return QSuccess;
}

void _qnet_cache_copy_common_data(QNetCache * qcache, QNetCacheEntryCommon * dst, QNetCacheEntryCommon * src) {
	dst->next = NULL;
	dst->prev = NULL;
	dst->timeout = src->timeout;
	return;
}
// iteration and search:
typedef QNetCacheEntryCommon * (*QNetCacheIterateFunc)(QNetCache * qcache, QNetCacheEntryCommon * entry, void * param);
QNetCacheEntryCommon * _qnet_cache_iterate(QNetCache * qcache, QNetCacheIterateFunc func, void * param) {
	QNetCacheEntryCommon * entry;
	QNetCacheEntryCommon * next;
	QNetCacheEntryCommon * ret = NULL;
	qnet_acquire_mutex(qcache->cache_mutex);

	uint64 now = qnet_get_cur_time();
	qcache->last_check_time = now;
	for (entry = qcache->first; entry != NULL; entry = next) {
		next = entry->next;
		if (now > entry->timeout) {
			_qnet_cache_remove_entry(qcache, entry);
		}
		ret = func(qcache, entry, param);
		if (ret != NULL) {
			break;
		}
		

	}
	qnet_release_mutex(qcache->cache_mutex);
	return ret;
}

QNetCacheEntryCommon * qnet_cache__iter_func__remove_invalids(QNetCache * qcache, QNetCacheEntryCommon * entry, void * param) {
	return NULL; // We just want to go over every entry and reomve the invalids.
}

QNetCacheEntryCommon * _qnet_cache__iter_func__copy(QNetCache * qcache, QNetCacheEntryCommon * entry, void * param) {
	QNetCacheEntryCommon ** entries_ptr = (QNetCacheEntryCommon **)param;
	QNetCacheEntryCommon  * temp = qnet_cache_create_entry(qcache);
	if (temp == NULL) { // not enough memory.
		_qnet_cache_entries_free(qcache, *entries_ptr);
		*entries_ptr = NULL;
		return (QNetCacheEntryCommon*)1; // error. stop iterate.
	}
	_qnet_cache_copy_common_data(qcache, temp, entry);
	if (FALSE == qcache->copy_func(qcache, temp, entry)) {
		_qnet_cache_entries_free(qcache, *entries_ptr);
		*entries_ptr = NULL;
		return (QNetCacheEntryCommon*)1; // error. stop iterate.
	}
	if (*entries_ptr == NULL) {
		*entries_ptr = temp;
	}
	else {
		(*entries_ptr)->next = temp;
		temp->prev = (*entries_ptr);
	}
	return NULL; // continue iterate.
}

QNetCacheEntryCommon * _qnet_cache__iter_func__find(QNetCache * qcache, QNetCacheEntryCommon * entry, void * param) {
	if (qcache->search_func(qcache, entry, param) == TRUE) return entry;
	return NULL;
}
QNetCacheEntryCommon * _qnet_cache__iter_func__find_and_remove(QNetCache * qcache, QNetCacheEntryCommon * entry, void * param) {
	if (qcache->search_func(qcache, entry, param) == TRUE) {
		_qnet_cache_remove_entry(qcache, entry);
		return (QNetCacheEntryCommon *)1;
	}
	return NULL;
}
QNetCacheEntryCommon * _qnet_cache__iter_func__find_and_remove_all(QNetCache * qcache, QNetCacheEntryCommon * entry, void * param) {
	if (qcache->search_func(qcache, entry, param) == TRUE) {
		_qnet_cache_remove_entry(qcache, entry);
	}
	return NULL;
}

QNetCacheEntryCommon * _qnet_cache__iter_func__check_ident(QNetCache * qcache, QNetCacheEntryCommon * entry, void * param) {
	QNetCacheEntryCommon * new_entry = (QNetCacheEntryCommon *)param;
	if (qcache->compare_func(qcache, entry, param) == 0) return entry;
	return NULL;
}

QResult qnet_cache_remove_invalids(QNetCache * qcache) {
	qnet_acquire_mutex(qcache->cache_mutex);
	_qnet_cache_iterate(qcache, qnet_cache__iter_func__remove_invalids, NULL);
	qnet_release_mutex(qcache->cache_mutex);
	return QSuccess;
}


QNetCacheEntryCommon * qnet_cache_get_copy(QNetCache * qcache) {
	QNetCacheEntryCommon * ret = NULL;
	uint32 ret_val = (uint32) _qnet_cache_iterate(qcache, _qnet_cache__iter_func__copy, (void*)(&ret));
	if (ret_val == 1) { // error!
		return NULL;
	}
	return ret; // The entries saved in this variable.
}

BOOL qnet_cache_find(QNetCache * qcache, void * user_define_struct) {
	return _qnet_cache_iterate(qcache, _qnet_cache__iter_func__find, user_define_struct)?TRUE:FALSE;
}
BOOL qnet_cache_find_and_remove(QNetCache * qcache, void * user_define_struct) {
	return _qnet_cache_iterate(qcache, _qnet_cache__iter_func__find_and_remove, user_define_struct)?TRUE:FALSE;
}
QResult qnet_cache_find_and_remove_all(QNetCache * qcache, void * user_define_struct) {
	return _qnet_cache_iterate(qcache, _qnet_cache__iter_func__find_and_remove_all, user_define_struct)?QSuccess:QFail;
}


