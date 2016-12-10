#include "../Common/Qube.h"
#include "../MemoryManager/heap.h"
#include "qstack.h"

QResult get_first_interface(QNetInterface * out) {
	// TODO.
	// This function shoule fill the 'out' variable.
	return QFail;
}

QResult qkr_main(KernelGlobalData *kgd) {
	QNetStack * qstk = kheap_alloc(sizeof(QNetStack));
	if (qnet_create_stack(qstk) == QFail) return QFail;
	
	QNetInterface * iface = kheap_alloc(sizeof(QNetInterface));
	if (get_first_interface(iface) == QFail) goto fail1;

	if (qnet_register_interface(qstk, iface) == QFail) goto fail2;

	iface->send_frame_func();
	return QSuccess;



fail2:
	kheap_free(iface);
fail1:
	kheap_free(qstk);
	return QFail;

}