#ifndef __QNET__LAYER2__
#define __QNET__LAYER2__
#include "qnetstack.h"
enum Layer2ProtocolType {
	L2RAW = 0,
	ETHERNET = 1,
};

struct _Layer2Listener {
	struct _Layer2Listener * next;
	void (*handle_frame)();
};

void qnet_layer2_handle_frame(QNetStack * qstk, QNetInterface * iface, QnetFrameToRecv * frame);





#endif //__QNET__LAYER2__

