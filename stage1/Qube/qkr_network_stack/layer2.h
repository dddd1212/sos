#ifndef __QNET__LAYER2__
#define __QNET__LAYER2__
#include "qstack.h"
enum Layer2ProtocolType {
	L2RAW = 0,
	ETHERNET = 1,
};
typedef struct {
	enum ProtocolType;
	HandlePacket(ProtocolType prev, Packet * pkt); // parse the protocol header and handle the data. may move the packet to a next layer protocol.
	PreparePacket(ProtocolType next, Packet * pkt);
	FixChecksum(Packet * pkt);

	
	// HandlePacket
	// 
} Layer2;

// Every protocol need to have function that create ProtocolContext.



#endif //__QNET__LAYER2__

