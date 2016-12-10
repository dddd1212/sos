#ifndef __QNET_IP__
#define __QNET_IP__
#include "qnetstack.h"
QResult qnet_ip_handle_packet(QNetStack * qstk, QNetInterface * iface, QnetFrameToRecv * frame);


#endif // __QNET_IP__