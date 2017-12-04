#include "qnet_iface.h"
#include "qnet_stats.h"

static g_iface_state_transitions[IFACE_STATE_NUM_OF_STATES] = { IFACE_STATE_GOING_DOWN, IFACE_STATE_UP, IFACE_STATE_DOWN, IFACE_STATE_GOING_UP };
static QNetProtocol PROTOCOLS[PROTO_NUM_OF_PROTOS] = { 
							{ PROTO_ETHER, qnet_ether_start_protocol, qnet_ether_stop_protocol },
							{ PROTO_ARP  , qnet_arp_start_protocol  , qnet_arp_stop_protocol },
							{ PROTO_IP   , qnet_ip_start_protocol   , qnet_ip_stop_protocol },
							{ PROTO_TCP  , qnet_tcp_start_protocol  , qnet_tcp_stop_protocol },
							{ PROTO_UDP  , qnet_udp_start_protocol  , qnet_udp_stop_protocol },
							{ PROTO_ICMP , qnet_icmp_start_protocol , qnet_icmp_stop_protocol } 
													  };

QResult qnet_init_iface(QNetStack * qstk, QNetInterface * iface, QNetRecvFrameFunc recv_frame_func, QNetSendFrameFunc send_frame_func, enum Protocols * protos, struct QNetOsInterface * os_iface) {
	iface->recv_frame_func = recv_frame_func;
	iface->send_frame_func = send_frame_func;
	iface->os_iface = os_iface;
	iface->next = NULL;
	iface->state = IFACE_STATE_DOWN;
	iface->protos = protos;
	iface->layer2_raw_listeners = NULL;

	// mutexes and event creation:
	if ((iface->tpool = qnet_tpool_create_pool()) == NULL) goto fail1;
	if ((iface->layer2_raw_listeners_mutex = qnet_create_mutex(TRUE)) == NULL) goto fail2;
	if ((iface->iface_mutex = qnet_create_mutex(TRUE)) == NULL) goto fail3;

	return QSuccess;
fail4:
	qnet_delete_mutex(iface->iface_mutex);
fail3:
	qnet_delete_mutex(iface->layer2_raw_listeners_mutex);
fail2:
	qnet_tpool_delete_pool(iface->tpool);
fail1:
	return QFail;

}


// This function should be called when the state of the iface is IFACE_STATE_GOING_UP or IFACE_STATE_DOWN
QResult qnet_iface_start(QNetStack * qstk, QNetInterface * iface) {
	QResult ret = QFail;
	QNetFrameListenerFuncParams * param;
	
	qnet_acquire_mutex(iface->iface_mutex);

	if (ret = qnet_iface_change_state(iface, IFACE_STATE_GOING_UP) != QSuccess) goto fail0;
	qnet_stats_reset_counters(qstk, iface);
	// Start the threads pool:
	if (qnet_tpool_start_pool(iface->tpool) != QSuccess) goto fail1;

	// Start the listener thread:
	if ((param = (QNetFrameListenerFuncParams *)qnet_malloc(sizeof(param))) == NULL) goto fail2; // The thread need to free this struct.
	param->qstk = qstk;
	param->iface = iface;
	if (QSuccess != qnet_tpool_start_new(iface->tpool, (QNetThreadFunc *)qnet_frame_listener_thread, (void *)param)) {
		qnet_free(param); // If we fail, we need to free it.
		goto fail2;
	}
	iface->protos;
	// Now we need to start the protocols threads:
	for (uint8 i = 0; i < PROTO_NUM_OF_PROTOS; i++) {
		if (IS_PROTO_ENABLE(i)) {
			if (QSuccess != PROTOCOLS[i].proto_start_func(qstk, iface)) goto fail2;
		}
		PROTO_MARK_UP(i);
	}
	
	
	qnet_iface_change_state(iface, IFACE_STATE_UP);
	ret = QSuccess;
	goto end;

fail2:
	if (QSuccess != qnet_tpool_stop_pool(iface->tpool)) goto fail0; // Don't change the state to state-down.
	// After the threads are down:
	for (uint8 i = 0; i < PROTO_NUM_OF_PROTOS; i++) {
		if (IS_PROTO_UP(i)) {
			PROTOCOLS[i].proto_stop_func(qstk, iface);
			PROTO_MARK_DOWN(i);
		}
	}
fail1:
	qnet_iface_change_state(iface, IFACE_STATE_DOWN);
fail0:
end:
	qnet_release_mutex(iface->iface_mutex);
	return ret;
}

QResult qnet_iface_stop(QNetStack * qstk, QNetInterface * iface) {
	QResult ret = QFail;
	qnet_acquire_mutex(iface->iface_mutex);
	if (QSuccess != qnet_iface_change_state(iface, IFACE_STATE_GOING_DOWN)) goto end;
	if (QSuccess != qnet_tpool_stop_pool(iface->tpool)) goto end;
	if (PROTO_ENABLE(PROTOB_ETHER)) {
		qnet_ether_start_protocol(qstk, iface);
	}
	if (PROTO_ENABLE(PROTOB_ARP)) {
		qnet_arp_start_protocol(qstk, iface);
	}
end:
	qnet_release_mutex(iface->iface_mutex);
	return ret;
}



////// internal fucntions:

BOOL qnet_iface_change_state(QNetInterface * iface, enum IfaceState new_state) {
	qnet_acquire_mutex(iface->iface_mutex);
	BOOL allow;
	switch (new_state) {
	case IFACE_STATE_DOWN:
		allow = (iface->state == IFACE_STATE_GOING_UP || iface->state == IFACE_STATE_GOING_DOWN);
		break;
	case IFACE_STATE_GOING_UP:
		allow = (iface->state == IFACE_STATE_DOWN);
		break;
	case IFACE_STATE_GOING_DOWN:
		allow = (iface->state == IFACE_STATE_UP);
		break;
	case IFACE_STATE_UP:
		allow = (iface->state == IFACE_STATE_GOING_UP);
		break;
	default:
		allow = FALSE;
	}

	if (allow) {
		iface->state = new_state;
	}
	qnet_release_mutex(iface->iface_mutex);
	return allow;
}

void qnet_frame_listener_thread(QNetFrameListenerFuncParams * param) {
	QNetFrameToRecv frame;

	QNetStack * qstk = param->qstk;
	QNetInterface * iface = param->iface;

	qnet_free((void*)param);
	qnet_iface_change_state(iface, IFACE_STATE_UP);
	// Start recieving packets:
	while (iface->state == IFACE_STATE_UP) {
		qnet_memset((uint8*)&frame, 0, sizeof(frame));
		iface->recv_frame_func(&frame, &iface->tpool->stop_event);
		qnet_stats_pkt_arrive(qstk, iface, &frame);
		if (qnet_handle_frame(qstk, iface, &frame) != QSuccess) {
			qnet_stats_pkt_drop(qstk, iface, &frame);
		};

		qnet_pkt_free_packet(frame.pkt);
	}
}

QResult qnet_handle_frame(QNetStack * qstk, QNetInterface * iface, QNetFrameToRecv * frame) {
	// First, give the frame to the raw listeners:
	qnet_acquire_mutex(iface->layer2_raw_listeners_mutex);
	for (Layer2Listener * i = iface->layer2_raw_listeners; i != NULL; i = i->next) {
		if (i->handle_frame(qstk, iface, frame) != QSuccess) return QFail;
	}
	qnet_release_mutex(iface->layer2_raw_listeners_mutex);

	// Then move it to the ethernet handler:
	if (PROTO_ENABLE(PROTOB_ETHER)) return qnet_ether_handle_frame(qstk, iface, &frame);
	return QSuccess;
}