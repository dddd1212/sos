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

// This is the common cache entry struct.
// You should extend it in your implementation (see doc. above).
struct _QNetCacheEntryCommon {
	struct _QNetCacheEntryCommon * next;
	struct _QNetCacheEntryCommon * prev;
	uint64 timeout; // in seconds
					// Here you should put the entry itself.
					// You need to create a struct with the CacheEntryCommon in it start and call the functions with cast.
};

#endif //__QNET_CACHE__
