#ifndef __QNET_DEFS__
#define __QNET_DEFS__

typedef void (*QnetThreadFunc)(void * param);


struct _Layer2Listener;
typedef struct _Layer2Listener Layer2Listener;


struct _ArpCache;
typedef struct _ArpCache ArpCache;

enum EtherType {
	ETHER_TYPE_ARP = 0x0608,
	ETHER_TYPE_IP = 0x800,
	ETHER_TYPE_IPV6 = 0xdd68,
};
#endif // __QNET_DEFS__
