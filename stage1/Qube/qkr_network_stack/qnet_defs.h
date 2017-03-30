#ifndef __QNET_DEFS__
#define __QNET_DEFS__

// This file contains basic declarations for the qnet library.
// Its things like forward declaration that have to be here,
// some protocols fields enums (like EtherType) that have to be here,
// etc.

// forward declerations.
struct QNetStack;
struct QNetInterface;

// Target of a packet in some level (ethernet, ip, etc.)
enum QNetTarget {
	QNET_TARGET_UNICAST = 0,
	QNET_TARGET_MULTICAST = 1,
	QNET_TARGET_BROADCAST = 2,
};

typedef void (*QNetThreadFunc)(void * param);



enum EtherType {
	ETHER_TYPE_ARP = 0x0608,
	ETHER_TYPE_IP = 0x800,
	ETHER_TYPE_IPV6 = 0xdd68,
};

enum ProtocolsBitmap {
	PROTOB_ETHER = 1 << 0,
	PROTOB_ARP = 1 << 1,
	PROTOB_IP = 1<<2,
	PROTOB_TCP = 1 << 3,
	PROTOB_UDP = 1<< 4,
	PROTOB_ICMP = 1<< 5,
};

#endif // __QNET_DEFS__
