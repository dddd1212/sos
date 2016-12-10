#ifndef __QNET_ARP__
#define __QNET_ARP__
#include "qnetstack.h"
QResult qnet_arp_handle_packet(QNetStack * qstk, QNetInterface * iface, QNetFrameToRecv * frame);


#endif // __QNET_ARP__