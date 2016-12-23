#include "qnet_arp.h"
#include "qnet_ether.h"

// ARP cache 4 functions implementation:
BOOL _qnet_arp_cache_entry_search(struct _QNetCache * qcache, QNetCacheEntryCommon * entry, void * user_defined_struct) {
	ArpCacheSearchEntity * search_entity = (ArpCacheSearchEntity *)user_defined_struct;
	ArpCacheEntry * arp_entry = (ArpCacheEntry *)entry;
	BOOL ip_match_found = FALSE;
	if (search_entity->match_ip && search_entity->ip == arp_entry->ip) {
		if (!search_entity->match_mac) { // match is found! fill the search_entity data:
			search_entity->found = TRUE;
			qnet_ether_copy_mac(search_entity->mac, arp_entry->mac);
			return TRUE;
		}
		// We still need to check the mac.
		ip_match_found = TRUE;
	}
	if (search_entity->match_mac && qnet_ether_compare_mac(search_entity->mac, arp_entry->mac)) {
		if (!search_entity->match_ip) { // found mac match!
			search_entity->ip = arp_entry->ip;
			search_entity->found = TRUE;
			return TRUE;
		}
		// if we here that mean that we need ip and mac match.
		if (ip_match_found) {
			search_entity->found = TRUE; // just mark as found.
		}
		return TRUE;
	}
	return FALSE;
}
uint32 _qnet_arp_cache_entry_compare(struct _QNetCache *qcache, QNetCacheEntryCommon * first, QNetCacheEntryCommon * second) {
	ArpCacheEntry * a_first = (ArpCacheEntry*)first;
	ArpCacheEntry * a_second = (ArpCacheEntry*)second;
	if (a_first->ip != a_second->ip) {
		return a_first->ip - a_second->ip;
	}
	return qnet_ether_compare_mac(a_first->mac, a_second->mac);

}
BOOL _qnet_arp_cache_entry_copy(struct _QNetCache *qcache, QNetCacheEntryCommon * dst, QNetCacheEntryCommon * src) {
	ArpCacheEntry * a_dst = (ArpCacheEntry*)dst;
	ArpCacheEntry * a_src = (ArpCacheEntry*)src;
	a_dst->ip = a_src->ip;
	qnet_ether_copy_mac(a_dst->mac, a_src->mac);
	return TRUE;
}
BOOL _qnet_arp_cache_entry_free(struct _QNetCache *qcache, QNetCacheEntryCommon * dst) {
	return TRUE;
}



/////////////////////////////
// This thread is the resolver thread. it sleeps until it needs to send a packet.


typedef struct {
	QNetStack * qstk;
	QNetInterface * iface;
} ArpSenderThreadParam;
void qnet_arp_sender_thread(void * param) {
	ArpSenderThreadParam * astp = (ArpSenderThreadParam *)param;
	QNetStack * qstk = astp->qstk;
	QNetInterface * iface = artp->iface;
	qnet_free(param);

	while (1) {
		// We are the only thread that wait on this event.
		qnet_wait_for_event(iface->arp_sender_event, 0xffffffff); // wait forever.
		if (TRUE == qnet_wait_for_event(iface->stop_event, 0)) { // The stop event is set! exit the thread...
			goto end;
		}
		// Check what is about:

	}

end:
	qnet_tpool_thread_finish(iface->tpool);
	return;
}

typedef struct {
	QNetStack * qstk;
	QNetInterface * iface;
} ArpWatchDogThreadParam;

void qnet_arp_watchdog_thread(void * param) {
	ArpWatchDogThreadParam * astp = (ArpWatchDogThreadParam *)param;
	QNetStack * qstk = astp->qstk;
	QNetInterface * iface = artp->iface;
	qnet_free(param);

	qnet_wait_for_event(iface->stop_event, 0xffffffff); // wait forever.
	// signal all of the arp events to make all the threads exit:
	qnet_set_event(iface->arp_sender_event);
	return;
}


QResult qnet_arp_resolve(QNetStack * qstk, QNetInterface * iface, uint32 ip, char * mac_out) {
	ArpCacheSearchEntity s;
	s.ip = ip;
	s.match_ip = TRUE;
	s.match_mac = FALSE;
	s.found = FALSE;
	if (iface->arp_thread == NULL) return QFail;
	if (FALSE == qnet_arp_cache_find(iface->arp_cache, &s)) { // need to send arp packet:
		
		qnet_arp_send_packet(QNetStack * qstk, QNetInterface * iface, QNetFrameToSend * frame);
	}

	
}
QResult qnet_arp_send_packet(QNetStack * qstk, QNetInterface * iface, QNetFrameToSend * frame) {
	
}


QResult qnet_arp_handle_packet(QNetStack * qstk, QNetInterface * iface, QNetFrameToRecv * frame) {
	return QFail;
}
