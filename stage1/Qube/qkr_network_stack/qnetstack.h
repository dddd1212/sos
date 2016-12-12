#ifndef __QNETWORK_STACK__H__
#define __QNETWORK_STACK__H__
#include "qnet_os.h"
#include "qnet_packet.h"
enum IfaceState {
	IFACE_STATE_DOWN = 0,
	IFACE_STATE_UP = 1,
};



typedef struct {
	Packet * pkt;
	BOOL need_to_free; // TRUE iff the QNetSendFrameFunc need to free the data after the call.
					   // if this is true, so after success call to QNetSendFrameFunc the this->pkt->(next->...)buf buffer will be 0.
} QnetFrameToSend;

typedef struct {
	Packet * pkt;
} QNetFrameToRecv;

typedef void (*QNetRecvFrameFunc)(QNetFrameToRecv * frame);
typedef void (*QNetSendFrameFunc)(QnetFrameToSend * frame);
// About packets memory management:
// To minimize the packets copies, we give the memory contorl to the stack-client. that means, that the operation system need to supply the 
// allocation functions. In that way, the recv_fram_func will return to us a pointer with the packet, and when we finish use the memory will "free" this memory.
// Also, to send packets, we can "allocate" a memory and use this memory to store the packet data. We may ask the send_frame_func function to free the packet memory
// when it finish the send process.


// This struct represent one network interface.
// recv_frame_func - callback that when called need to wait until packet is ready and then return with the packet.
// send_frame_func - callback that send a frame on the network interface.
// os_iface - struct of the os specific interface parameters.
struct _QNetInterface {
	struct _QNetInfterface * next;
	QNetRecvFrameFunc recv_frame_func;
	QNetSendFrameFunc send_frame_func;
	struct QNetOsInterface * os_iface;
	enum IfaceState state;
	
	QNetMutex * layer2_raw_listeners_mutex;
	Layer2Listener * layer2_raw_listeners;

	// statistics:
	uint32 recv_pkts_count;
	uint32 recv_bytes_count;

	// Per protocol fields:
	// ARP:
	ArpCache * arp_cache;
};
typedef struct _QNetInterface QNetInterface;




// This struct is the general context of the network stack. It contains everything.
typedef struct {
	QNetMutex * ifaces_mutex;
	QNetInterface * ifaces;
} QNetStack;

typedef struct {
	QNetStack * qstk;
	QNetInterface * iface;
} QNetFrameListenerFuncParams;
void qnet_frame_listener_func(QNetFrameListenerFuncParams * param);
QResult qnet_create_stack(QNetStack * qstk);
QResult qnet_init_iface(QNetInterface * iface, QNetRecvFrameFunc recv_frame_func, QNetSendFrameFunc send_frame_func, struct QNetOsInterface * os_iface);
QResult qnet_register_interface(QNetStack * qstk, QNetInterface * iface);





#endif // __QNETWORK_STACK__H__