#include "qnetstack.h"
#include "qnet_ether.h"
#include "qnet_stats.h"
#include "qnet_os.h"


QResult qnet_create_stack(QNetStack * qstk) {
	if (QSuccess != (qstk->ifaces_mutex = qnet_create_mutex(TRUE))) return QFail;
	qstk->ifaces = NULL;
	return QSuccess;
}

QResult qnet_register_interface(QNetStack * qstk, QNetInterface * iface, BOOL to_start) {
	
	// Register the interface in the interfaces list:
	qnet_acquire_mutex(qstk->ifaces_mutex);
	iface->next = qstk->ifaces->next;
	qstk->ifaces = iface;
	qnet_iface_change_state(IFACE_STATE_GOING_UP);
	qnet_release_mutex(qstk->ifaces_mutex);
	
	if (to_start) {
		QResult ret = qnet_iface_start(iface);
		if (ret != QSuccess) {
			qnet_deregister_interface(qstk, iface);
			return QFail;
		}
	}
	return QSuccess;
}

QResult qnet_deregister_interface(QNetStack * qstk, QNetInterface * iface) {
	return QFail;
}

