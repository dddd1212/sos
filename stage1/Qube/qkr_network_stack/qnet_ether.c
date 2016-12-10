#include "qnet_ether.h"
#include "qnet_packet.h"
#include "qnet_os.h"
#include "qnet_arp.h"
#include "qnet_ip.h"
#include "qnet_ipv6.h"

QResult qnet_register_layer2_listener(QNetStack * qstk, QNetInterface * iface, Layer2Listener * listener) {
	qnet_acquire_mutex(iface->layer2_raw_listeners_mutex);
	iface->layer2_raw_listeners->next = listener->next;
	iface->layer2_raw_listeners = listener;
	qnet_release_mutex(iface->layer2_raw_listeners_mutex);
	return QSuccess;
}

QResult qnet_deregister_layer2_listener(QNetStack * qstk, QNetInterface * iface, Layer2Listener * listener) {
	QResult ret;
	qnet_acquire_mutex(iface->layer2_raw_listeners_mutex);
	if (listener == iface->layer2_raw_listeners) {
		iface->layer2_raw_listeners = iface->layer2_raw_listeners->next;
		listener->next = NULL;
		ret = QSuccess;
		goto end;
	}
	Layer2Listener * prev = iface->layer2_raw_listeners;
	for (Layer2Listener * i = prev->next; i != NULL; prev = i, i = i->next) {
		if (i == listener) {
			prev->next = i->next;
			listener->next = NULL;
			ret = QSuccess;
			goto end;
		}
	}
	ret = QFail;
	goto end;
end:
	qnet_release_mutex(iface->layer2_raw_listeners_mutex);
	return ret;
}
QResult qnet_ether_handle_frame(QNetStack * qstk, QNetInterface * iface, QNetFrameToRecv * frame) {
	// First, give the frame to the raw listeners:
	qnet_acquire_mutex(iface->layer2_raw_listeners_mutex);
	for (Layer2Listener * i = iface->layer2_raw_listeners; i != NULL; i = i->next) {
		if (i->handle_frame(qstk, iface, frame) != QSuccess) return QFail;
	}
	qnet_release_mutex(iface->layer2_raw_listeners_mutex);

	// Then, move the frame the next layer:
	Packet * pkt = frame->pkt;
	uint32 pkt_size = qnet_pkt_get_size(pkt);
	if (pkt_size < ETHER_SIZE) return QFail;
	if (qnet_pkt_same_buf_arrange(pkt, ETHER_SIZE) != QSuccess) return QFail;
	Ethernet * ether = (Ethernet *) qnet_pkt_get_ptr(pkt, 0);
	enum EtherType ether_type = qnet_swap16(ether->ether_type);
	if (qnet_pkt_consume_bytes(pkt, ETHER_SIZE) != QSuccess) return QFail;
	switch (ether_type) {
	case ETHER_TYPE_ARP:
		return qnet_arp_handle_packet(qstk, iface, frame);
	case ETHER_TYPE_IP:
		return qnet_ip_handle_packet(qstk, iface, frame);
	case ETHER_TYPE_IPV6:
		return qnet_ipv6_handle_packet(qstk, iface, frame);
	default:
		return QFail;
	}
	// can't reach here:
	return QFail;

}