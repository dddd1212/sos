#ifndef __QNET_ARP__
#define __QNET_ARP__
#include "qnetstack.h"
#include "qnet_cache.h"

struct _ArpCache;
typedef struct _ArpCache ArpCache;

// defines the cache struct and 4 functions:
struct _ArpCacheEntry {
	struct _QNetCacheEntryCommon common;
	// The entry:
	uint32 ip;
	uint8 mac[6];
};
typedef struct _ArpCacheEntry ArpCacheEntry;
struct _ArpCacheSearchEntity {
	uint32 ip;
	uint8 mac[6];
	BOOL match_ip;
	BOOL match_mac;
	BOOL found;
};
typedef struct _ArpCacheSearchEntity ArpCacheSearchEntity;

typedef struct {
	uint16 HTYPE; // Hardware type
	uint16 PTYPE; // Protocol type
	uint8 HLEN; // Hardware address length
	uint8 PLEN; // Protocol address length
	uint16 OPER; // request or response
	uint8 SHA[6]; // sender hardware address
	uint32 SPA; // sender protocol address
	uint8 THA[6]; // target hardware address
	uint32 TPA; //target protocol address
} ArpPacket;


typedef struct {
	int dodo;
} ArpProtocol;

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

QResult qnet_arp_send_packet(QNetStack * qstk, QnetInterface * iface, QNetFrameToSend * frame);
QResult qnet_arp_handle_packet(QNetStack * qstk, QNetInterface * iface, QNetFrameToRecv * frame);


#endif // __QNET_ARP__