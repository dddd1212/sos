#ifndef __QNET_ARP__
#define __QNET_ARP__
#include "qnetstack.h"

struct _ArpCacheEntry {
	struct _ArpCacheEntry * next;
	uint32 age; // in seconds
	uint32 invalidate_time; // in seconds
	// The entry:
	uint32 ip;
	char mac[6];
};
typedef struct _ArpCacheEntry ArpCacheEntry;

struct _ArpCache {
	uint32 max_size;
	uint32 min_invalidate_interval; // in seconds
	uint32 actual_size;
	struct _ArpCacheEntry * entries;
};

QResult qnet_arp_create_cache(ArpCache * arp_cache);
QResult qnet_arp_destroy_cache(ArpCache * arp_cache);

QResult qnet_arp_get_mac(uint32 ip);

QResult qnet_arp_add_cache_entry(QNetStack * qstk, QNetInterface * iface, QNetFrameToRecv * frame);

QResult qnet_arp_handle_packet(QNetStack * qstk, QNetInterface * iface, QNetFrameToRecv * frame);


#endif // __QNET_ARP__