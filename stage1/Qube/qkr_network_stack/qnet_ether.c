#include "qnet_ether.h"
#include "qnet_packet.h"
#include "qnet_os.h"
#include "qnet_arp.h"
#include "qnet_ip.h"
#include "qnet_ipv6.h"
#include "qnet_iface.h"

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

// The function wrap the frame with ether layer and send it.
// The frame cur pointer need to point to the start of the next layer.
QResult qnet_ether_send_frame(QNetStack * qstk, QnetInterface * iface, QNetFrameToSend * frame) {
	if (QSuccess != qnet_pkt_add_bytes_to_start(frame->pkt, ETHER_SIZE)) return QFail;
	if (QSuccess != qnet_pkt_go_back_bytes(frame->pkt, ETHER_SIZE)) return QFail;
	if (QSuccess != qnet_pkt_same_buf_arrange(frame->pkt, ETHER_SIZE)) return QFail;
	Ethernet * hdr = (Ethernet*)qnet_pkt_get_ptr(0);
	qnet_ether_copy_mac(hdr->src_mac, frame->cpf.src_mac);
	qnet_ether_copy_mac(hdr->dst_mac, frame->cpf.dst_mac);
	hdr->ether_type = frame->cpf.ether_type;
	return iface->send_frame_func(&frame);
}
QResult qnet_ether_handle_frame(QNetStack * qstk, QNetInterface * iface, QNetFrameToRecv * frame) {
	Packet * pkt = frame->pkt;
	uint32 pkt_size = qnet_pkt_get_size(pkt);
	if (pkt_size < ETHER_SIZE) return QFail;
	if (qnet_pkt_same_buf_arrange(pkt, ETHER_SIZE) != QSuccess) return QFail;
	Ethernet * ether = (Ethernet *) qnet_pkt_get_ptr(pkt, 0);
	qnet_ether_copy_mac(frame->cpf.dst_mac, ether->dst_mac);
	qnet_ether_copy_mac(frame->cpf.src_mac, ether->src_mac);
	enum EtherType ether_type = qnet_swap16(ether->ether_type);
	frame->cpf.ether_type = ether_type;

	if (ether->dst_mac[0] & 1) {
		if (qnet_ether_compare_mac(ether->dst_mac, (uint8*)"\xff\xff\xff\xff\xff\xff") == 0) {
			frame->cpf.l2_target = QNET_TARGET_BROADCAST;
		} else { // multicast. we don't support that yet.
			return QFail;
		}
	}
	else { // unicast
		if (qnet_ether_compare_mac(ether->dst_mac, iface->mac_address) != 0) {
			return QFail; // not to us.
		}
	}
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


void qnet_ether_copy_mac(uint8 * dst, uint8 * src) {
	qnet_memcpy(dst, src, 6);
	return;
}
uint32 qnet_ether_compare_mac(uint8 * first, uint8 * second) {
	return qnet_memcmp(first, second, 6);
}


QResult qnet_ether_start_protocol(QNetStack * qstk, QNetInterface * iface) {
	// nothing to do...
	iface->protos_up |= 
	return QSuccess;
}