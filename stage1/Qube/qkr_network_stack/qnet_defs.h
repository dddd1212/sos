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

enum Protocols {
	PROTO_ETHER = 0,
	PROTO_ARP = 1,
	PROTO_IP = 2,
	PROTO_TCP = 3,
	PROTO_UDP = 4,
	PROTO_ICMP = 5,
	PROTO_NUM_OF_PROTOS = 6,
};

#endif // __QNET_DEFS__
