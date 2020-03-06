#ifndef __QNET_IP__
#define __QNET_IP__
#include "qnetstack.h"
QResult qnet_ip_handle_packet(QNetStack * qstk, QNetInterface * iface, QNetFrameToRecv * frame);
QResult qnet_ip_start_protocol(QNetStack * qstk, QNetInterface * iface);
QResult qnet_ip_stop_protocol(QNetStack * qstk, QNetInterface * iface);


#endif // __QNET_IP__