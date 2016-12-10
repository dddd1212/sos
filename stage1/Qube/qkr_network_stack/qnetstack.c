#include "qnetstack.h"
#include "qnet_ether.h"
#include "qnet_stats.h"
QResult qnet_create_stack(QNetStack * qstk) {
	if (QSuccess != (qstk->ifaces_mutex = qnet_create_mutex(TRUE))) return QFail;
	qstk->ifaces = NULL;
	return QSuccess;
}
QResult qnet_init_iface(QNetInterface * iface, QNetRecvFrameFunc recv_frame_func, QNetSendFrameFunc send_frame_func, struct QNetOsInterface * os_iface) {
	iface->recv_frame_func = recv_frame_func;
	iface->send_frame_func = send_frame_func;
	iface->os_iface = os_iface;
	iface->next = NULL;
	iface->state = IFACE_STATE_DOWN;
	
	if ((iface->layer2_raw_listeners_mutex = qnet_create_mutex(TRUE)) == NULL) return QFail;
	iface->layer2_raw_listeners = NULL;
	
	return QSuccess;
}

QResult qnet_register_interface(QNetStack * qstk, QNetInterface * iface) {
	QNetFrameListenerFuncParams * param;
	// Register the interface in the interfaces list:
	qnet_acquire_mutex(qstk->ifaces_mutex);
	iface->next = qstk->ifaces->next;
	qstk->ifaces = iface;
	qnet_release_mutex(qstk->ifaces_mutex);
	// Start the listener thread:
	param = (QNetFrameListenerFuncParams *) qnet_alloc(sizeof(param)); // The thread need to free this struct.
	param->qstk = qstk;
	param->iface = iface;
	qnet_start_thread((QnetThreadFunc *) qnet_frame_listener_func, (void *) param);

	return QSuccess;
}

void qnet_frame_listener_func(QNetFrameListenerFuncParams * param) {
	QNetFrameToRecv frame;
	
	QNetStack * qstk = param->qstk;
	QNetInterface * iface = param->iface;
	
	qnet_free((void*)param);
	iface->state = IFACE_STATE_UP;
	// Start recieving packets:
	while (iface->state == IFACE_STATE_UP) {
		frame.pkt = NULL;
		iface->recv_frame_func(&frame);
		qnet_stats_pkt_arrive(qstk, iface, &frame);
		qnet_ether_handle_frame(qstk, iface, &frame);
	}
}