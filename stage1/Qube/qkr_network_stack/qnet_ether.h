#ifndef __QNET__LAYER2__
#define __QNET__LAYER2__
#include "qnetstack.h"


typedef struct {
	uint8 dst_mac[6];
	uint8 src_mac[6];
	uint16 ether_type;
}Ethernet;

#define ETHER_SIZE 14

enum Layer2ProtocolType {
	L2RAW = 0,
	ETHERNET = 1,
};



QResult qnet_ether_handle_frame(QNetStack * qstk, QNetInterface * iface, QNetFrameToRecv * frame);
void qnet_ether_copy_mac(uint8 * dst, uint8 * src);
uint32 qnet_ether_compare_mac(uint8 * first, uint8 * second);




#endif //__QNET__LAYER2__

