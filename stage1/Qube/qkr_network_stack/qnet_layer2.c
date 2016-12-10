#include "qnet_layer2.h"

void qnet_layer2_handle_frame(QNetStack * qstk, QNetInterface * iface, QnetFrameToRecv * frame) {
	// First, give the frame to the raw listeners:
	for (Layer2Listener * i = iface->layer2_raw_listeners; i != NULL; i = i->next) {
		i->handle_frame(qstk, iface, frame);
	}

	// Then, move the frame the next layer:
}