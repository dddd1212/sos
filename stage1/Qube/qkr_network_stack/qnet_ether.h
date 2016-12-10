#ifndef __QNET__LAYER2__
#define __QNET__LAYER2__
#include "qnetstack.h"
enum EtherType {
	ETHER_TYPE_ARP = 0x0608,
	ETHER_TYPE_IP = 0x800,
	ETHER_TYPE_IPV6 = 0xdd68,
};

typedef struct {
	char dst_mac[6];
	char src_mac[6];
	uint16 ether_type;
}Ethernet;

#define ETHER_SIZE 14

enum Layer2ProtocolType {
	L2RAW = 0,
	ETHERNET = 1,
};

struct _Layer2Listener {
	struct _Layer2Listener * next;
	void (*handle_frame)();
};

void qnet_ether_handle_frame(QNetStack * qstk, QNetInterface * iface, QnetFrameToRecv * frame);





#endif //__QNET__LAYER2__

