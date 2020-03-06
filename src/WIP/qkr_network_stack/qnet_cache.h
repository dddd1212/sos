#ifndef __QNET_CACHE__
#define __QNET_CACHE__

#include "qnetstack.h"
// This file contains a generic cache.
// To use it you need to create a struct that will contains in the head the _QNetCacheEntryCommon struct:
// struct my_cache {
//	struct _QNetCacheEntryCommon;
//  .
//  . your cache entry fields.
//  .
// }
// 
// To create a cache, you will need to supply 4 functions:
// 1. search function - function that get 3 parameters: The cache, 
//														An entry in the cache,
//														A pointer to some user-data.
//						The function need to use the user-data to determine if the entry is fit to this data.
//					    For example, in ARP cache, the function may treat the user-data as ip_address and check if the entry has this ip address.
//						The function need to return TRUE iff the entry fit.
//
// 2. compare function - function that get as parameters: The cache,
//														  2 cache entries.
//						 The function need to compare the entries. The function need define order between 2 cache entries. If first < second, return negative value.
//																														   If first > second, return positive value.
//																														   If first == seconde, return 0.
// 3. copy function - function that get already allocated 2 entries structs (with the size of your extended _QNetCacheEntryCommon struct) and copy all the rellevant data
//					  to your extension struct from the old one to the new one.
//
// 4. free function - function that need to free entry. COUTION: the function not need to actually free the entry strcut, but just pointers in the
//					  struct that need to be free. If your implementation just add some variables to the QNetCacheEntryCommon, the function should do nothing.
//

struct _QNetCache;

// This is the common cache entry struct.
// You should extend it in your implementation (see documentation above).
struct _QNetCacheEntryCommon {
	struct _QNetCacheEntryCommon * next;
	struct _QNetCacheEntryCommon * prev;
	uint64 timeout; // in seconds
					// Here you should put the entry itself.
					// You need to create a struct with the CacheEntryCommon in it start and call the functions with cast.
};
typedef struct _QNetCacheEntryCommon QNetCacheEntryCommon;

// The 4 funtions that the user need to implement (see documentation above).
// Also, see the documentation of the functions qnet_cache_find* to see how to use the user_defined_struct.
typedef BOOL(*QNetCacheEntrySearchFunc)(struct _QNetCache *qcache, QNetCacheEntryCommon * entry, void * user_defined_struct);
typedef uint32(*QNetCacheEntryCompareFunc)(struct _QNetCache *qcache, QNetCacheEntryCommon * first, QNetCacheEntryCommon * second);
typedef BOOL(*QNetCacheEntryCopyFunc)(struct _QNetCache *qcache, QNetCacheEntryCommon * dst, QNetCacheEntryCommon * src); // copy all the user struct.
typedef BOOL(*QNetCacheEntryFreeFunc)(struct _QNetCache *qcache, QNetCacheEntryCommon * dst); // free the user data of the entry.

// This struct is the 
struct _QNetCache {
	uint32 max_size; // maximum number of elements in the cache.
	uint32 actual_size; // current number of elements.
	uint64 last_check_time; // last time we iterate the cache (and try to remove invalid records).
							// This parameter does nothing right now.
	uint32 real_entry_size; // The size of the user QNetCacheEntryCommon extended struct.
	// 4 user functions to search entry, compare entries, copy entry and free entry.
	QNetCacheEntrySearchFunc search_func;
	QNetCacheEntryCompareFunc compare_func;
	QNetCacheEntryCopyFunc copy_func;
	QNetCacheEntryFreeFunc free_func;
	
	// The list of the entries:
	struct _QNetCacheEntryCommon * first;
	struct _QNetCacheEntryCommon * last;
	
	// General mutex to hold when we use the cache.
	QNetMutex * cache_mutex;
};
typedef struct _QNetCache QNetCache;


// functions definitions:

// creation functions:
	// This function create an empty cache. You should tell the function what is the size of your
	// extended entry struct, and also pass the 4 user-functions.
	// you also need to pass the wanted max_size (number of entries) allowed in the cache.
	QNetCache * qnet_cache_create(uint32 real_entry_size, QNetCacheEntrySearchFunc search_func,
		QNetCacheEntryCompareFunc compare_func,
		QNetCacheEntryCopyFunc copy_func,
		QNetCacheEntryFreeFunc free_func,
		uint32 max_size);

	// Destroies an existing cache.
	// It is safe to call to this function from anyware outside the cache functions.
	// Don't call to this function inside one of the 4 user-functions (compre, copy, free and search).
	QResult qnet_cache_destroy(QNetCache * qcache);

	// This function creates an entry (allocate it).
	// The type of the return entry is QNetCacheEntryCommon, but it really alloc the extended struct,
	// so it safe to cast it to the user extended struct.
	// The function will return NULL if error occured.
	QNetCacheEntryCommon * qnet_cache_create_entry(QNetCache * qcache);

// insetion functions:
	// The function insert entry to an existing cache.
	// The entry will be valid for 'timeout' seconds.
	// If the entry is already in the cache (we check this by the user-defined compare function)
	// we replace it by the new entry.
	QResult qnet_cache_add_entry(QNetCache * qcache, QNetCacheEntryCommon * entry, uint32 timeout);

	// The function removes an entry from the qcache.
	// CAUTION: this is internal function and it not thread-safe!
	//			you shouldn't call this function not from function that take the cache general mutex,
	//			because there is no check in the implementation of the function that the entry does exists in
	//			qcache. the function assumes that the entry exists!
	void _qnet_cache_remove_entry(QNetCache * qcache, QNetCacheEntryCommon * entry);
// additional helper functions (not API functions. you should not use them!
	// the function free list of entries.
	// The function free every extended struct with the user-defined free function and then
	// free the entry struct itself.
	QResult _qnet_cache_entries_free(QNetCache * qcache, QNetCacheEntryCommon * entries);
	
	// The function copies the common fields (the base struct QNetCacheEntryCommon) from pre-allocated struct 'src'
	// to pre-allocated struct 'dst'.
	void _qnet_cache_copy_common_data(QNetCache * qcache, QNetCacheEntryCommon * dst, QNetCacheEntryCommon * src);

// iteration and search:
	// Function header of iterate function. see documention above.
	typedef QNetCacheEntryCommon * (*QNetCacheIterateFunc)(QNetCache * qcache, QNetCacheEntryCommon * entry, void * param);
	
	// This function is general function to iterate over the cache.
	// The function get the qcache itself, and the iterate function and param.
	// The function iterate on the cache (and remove invalids records), and for every entry,
	// It calles to the 'func' function with the 'qcache', the current entry, and the 'param' given to this function.
	// 'func' needs to return NULL or pointer to the current entry.
	// When not-NULL is returnd, the iterator stop iterate.
	// The function doesn't need the entry that it pass to the user function 'func'. that means,
	// that the user can delete it, or change it.
	// The iterator starts from the head and move to the tail. keep this in mind when you do complicated
	// stuff in the iterator's function.
	// You can use in 'func' this practice: return the number 1 to indicate error and stop the iteration.
	// The function handle this properly. If you use this practice, you have to check the pointer in the callee.
	QNetCacheEntryCommon * _qnet_cache_iterate(QNetCache * qcache, QNetCacheIterateFunc func, void * param);

	// iterator functions:
	// just remove the invalid records. the function just returns NULL.
	QNetCacheEntryCommon * _qnet_cache__iter_func__remove_invalids(QNetCache * qcache, QNetCacheEntryCommon * entry, void * param);
	

	// The function use to copy the entire cache.
	// The function itself get QNetCacheEntryCommon ** as 'param' and add to this list an entry.
	// The function could return the number 1 if error occured.
	QNetCacheEntryCommon * _qnet_cache__iter_func__copy(QNetCache * qcache, QNetCacheEntryCommon * entry, void * param);

	// These functions checks, by calling the user function 'search_func' if 'entry' is a record that fits to the 'param' variable.
	// The 'param' variable could be what ever you decide that your search_func can parse and understand. the 'void * param' parameter
	// Will pass to the 'search_func' as void * user_defined_struct.
	// find - returns the first entry that fit the void * param.
	// find_and_remove - just remove the first entry that fit. The return value could be NULL (not found).
	//															The return value could be 1 (an error accoured).
	// find_and_remove_all - removes every entry that fit. this function returns always NULL.
	QNetCacheEntryCommon * _qnet_cache__iter_func__find(QNetCache * qcache, QNetCacheEntryCommon * entry, void * param);
	QNetCacheEntryCommon * _qnet_cache__iter_func__find_and_remove(QNetCache * qcache, QNetCacheEntryCommon * entry, void * param);
	QNetCacheEntryCommon * _qnet_cache__iter_func__find_and_remove_all(QNetCache * qcache, QNetCacheEntryCommon * entry, void * param);
	
	// check if 2 entries are the same. the 'param' here is QNetCacheEntryCommon * other.
	// The function use the user-defined 'compare_func' to compare between the 2 entries.
	QNetCacheEntryCommon * _qnet_cache__iter_func__check_ident(QNetCache * qcache, QNetCacheEntryCommon * entry, void * param);

// functions to iterate with:
	QResult qnet_cache_remove_invalids(QNetCache * qcache);
	QNetCacheEntryCommon * qnet_cache_get_copy(QNetCache * qcache);

	
	BOOL qnet_cache_find(QNetCache * qcache, void * user_define_struct); // TRUE - entry found.
	BOOL qnet_cache_find_and_remove(QNetCache * qcache, void * user_define_struct); // TRUE - entry found.
	QResult qnet_cache_find_and_remove_all(QNetCache * qcache, void * user_define_struct); // QSuccess - no error.





#endif //__QNET_CACHE__
