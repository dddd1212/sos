#ifndef __QNET_ARP__
#define __QNET_ARP__
#include "qnetstack.h"
#include "qnet_cache.h"
// defines the cache struct and 4 functions:
struct _ArpCacheEntry {
	struct _QNetCacheEntryCommon;
	// The entry:
	uint32 ip;
	char mac[6];
};
typedef struct _ArpCacheEntry ArpCacheEntry;
BOOL _qnet_arp_cache_entry_search(struct _QNetCache * qcache, QNetCacheEntryCommon * entry, void * user_defined_struct);
uint32 _qnet_arp_cache_entry_compare(struct _QNetCache *qcache, QNetCacheEntryCommon * first, QNetCacheEntryCommon * second);
BOOL _qnet_arp_cache_entry_copy(struct _QNetCache *qcache, QNetCacheEntryCommon * dst, QNetCacheEntryCommon * src); 
BOOL _qnet_arp_cache_entry_free(struct _QNetCache *qcache, QNetCacheEntryCommon * dst); 
// defines for easy use:
#define qnet_arp_cache_create(search_func,compare_func,copy_func,free_func,max_size) \
				qnet_cache_create(sizeof(ArpCacheEntry), search_func, compare_func, copy_func, free_func, max_size)

#define qnet_arp_cache_destroy(qcache) qnet_cache_destroy(qcache)
#define qnet_arp_cache_create_entry(qcache) qnet_cache_create_entry(qcache)
#define qnet_arp_cache_add_entry(qcache, entry, timeout) qnet_cache_add_entry(qcache, (QNetCacheEntryCommon *) entry, timeout)

// functions to iterate with:
#define qnet_arp_cache_get_copy(qcache) qnet_arp_cache_get_copy(qcache)
#define qnet_arp_cache_find(qcache, search_entity) qnet_cache_find(qcache, (void *) search_entity)
#define qnet_arp_cache_find_and_remove(qcache, search_entity) qnet_cache_find_and_remove(qcache, (void * )search_entity)
#define qnet_arp_cache_find_and_remove_all(qcache, search_entity) qnet_cache_find_and_remove_all(qcache, (void * )search_entity)


QResult qnet_arp_handle_packet(QNetStack * qstk, QNetInterface * iface, QNetFrameToRecv * frame);


#endif // __QNET_ARP__