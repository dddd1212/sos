#include "qnet_arp.h"
#include "qnet_ether.h"
#include "qnet_iface.h"
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
	//return qnet_ether_compare_mac(a_first->mac, a_second->mac);
	return 0;
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
} ArpResolverThreadParam;

void qnet_arp_resolver_thread(void * param) {
	ArpResolverThreadParam * astp = (ArpResolverThreadParam *)param;
	QNetStack * qstk = astp->qstk;
	QNetInterface * iface = astp->iface;
	qnet_free(param);

	while (1) {
		// We are the only thread that wait on this event.
		qnet_wait_for_event(iface->arp.arp_sender_event, 0xffffffff); // wait forever.
		if (TRUE == qnet_wait_for_event(iface->stop_event, 0)) { // The stop event is set! exit the thread...
			goto end;
		}
		// Send all the timeouted packet:

		for (ArpResolvePacket * prev = NULL, *i = iface->arp.res_pkts; i != NULL; prev = i, i = i->next) {
			// Check if we need to get the 
		}

	}

end:
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
	ArpResolvePacket arp;
	
	s.ip = ip;
	s.match_ip = TRUE;
	s.match_mac = FALSE;
	s.found = FALSE;
	if (iface->arp.is_up == FALSE) return QFail;
	// Shortcut:
	if (TRUE == qnet_arp_cache_find(iface->arp.arp_cache, &s)) {
		qnet_memcpy(mac_out, s.mac, 6);
		return QSuccess;
	}
	
	// Create the event:
	arp.ev = qnet_create_event(TRUE);
	if (arp.ev == NULL) {
		return QFail;
	}
	qnet_reset_event(arp.ev);
	arp.ip = ip;
	if (QFail == add_to_res_pkts(iface, &arp)) { // Too many resolvers...
		qnet_delete_event(arp.ev);
		return QFail;
	};

	for (uint8 i = 0; i <= ARP_RETRIES; i++) {
		// Search the entry in the cache:
		if (FALSE == qnet_arp_cache_find(iface->arp.arp_cache, &s)) { // need to send arp packet:														  
			if (i < ARP_RETRIES) {
				_qnet_arp_send_query(qstk, iface, ip);
				if (TRUE == qnet_wait_for_event(arp.ev, ARP_TIMEOUT)) {
					// Check if it is a 
					if ()
					break;
				
				}

			}
		} else { // We found match! 
			break;
		}
		// fail here:
		// TODO...
	}
	qnet_memcpy(mac_out, s.mac, 6);
	// clean:
	rem_from_res_pkts(iface, &arp);
	qnet_delete_event(arp.ev);
	return QSuccess;
	
	}
	
}
QResult qnet_arp_send_packet(QNetStack * qstk, QNetInterface * iface, QNetFrameToSend * frame) {
	
}


QResult qnet_arp_handle_packet(QNetStack * qstk, QNetInterface * iface, QNetFrameToRecv * frame) {
	return QFail;
}

/// The start and stop routines:

QResult qnet_ether_start_protocol(QNetStack * qstk, QNetInterface * iface) {
	// Start the threads:
	ArpResolverThreadParam * a = qnet_alloc(sizeof(ArpResolverThreadParam));
	if (a == NULL) goto fail;
	a->iface = iface;
	a->qstk = qstk;
	if (QSuccess != qnet_tpool_start_new(iface->tpool, (QNetThreadFunc *)qnet_arp_resolver_thread, (void *)a)) {
		qnet_free(a);
		goto fail;
	}
	return QSuccess;

fail:
	return QFail;
}
QResult qnet_ether_stop_protocol(QNetStack * qstk, QNetInterface * iface) {
	// The threads are going down automatilcaly.
	// Our function is called after the threads are already down.
	return QSuccess;
}

