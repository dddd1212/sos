#ifndef __QNET_IFACE_H__
#define __QNET_IFACE_H__
#include "qnet_os.h"
#include "qnet_defs.h"
#include "qnet_os.h"

#include "qnet_packet.h"
#include "qnet_threads_pool.h"
// protocols:
#include "qnet_ether.h"
#include "qnet_arp.h"
#include "qnet_ip.h"
#include "qnet_ipv6.h"
#include "qnet_udp.h"
#include "qnet_tcp.h"
#include "qnet_icmp.h"
////////////////////////////// General packets declarations: /////////////////////////////////////

// Every Packet that recieved / sent has some typical fields that we use in the general packet 
// struct to simplify things, although not every packet contains the all of the fields:
typedef struct {
	uint8 src_mac[6];
	uint8 dst_mac[6];
	enum EtherType ether_type; // ip / arp / etc ...
	enum QNetTarget l2_target;

	uint32 src_ip;
	uint32 dst_ip;
	uint8 proto; // tcp / udp / icmp ...
	enum QNetTarget l3_target;

	uint16 src_port;
	uint16 dst_port;
} QNetCommonPacketFields;

typedef struct {
	Packet * pkt;
	BOOL need_to_free; // TRUE iff the QNetSendFrameFunc need to free the data after the call.
					   // if this is true, so after success call to QNetSendFrameFunc the 
					   // this->pkt->(next->...)buf buffer will be 0.
	QNetCommonPacketFields cpf;
} QNetFrameToSend;

typedef struct {
	Packet * pkt;
	QNetCommonPacketFields cpf;
} QNetFrameToRecv;

// About packets memory management:
// To minimize the packets copies, we give the memory contorl to the stack-client. that means, that the operation system need to supply the 
// allocation functions. In that way, the recv_fram_func will return to us a pointer with the packet, and when we finish use the memory will "free" this memory.
// Also, to send packets, we can "allocate" a memory and use this memory to store the packet data. We may ask the send_frame_func function to free the packet memory
// when it finish the send process.

/////////////////////////////////////////////////////////////////////////////////////





//////////////////// THE INTERFACE STRUCT ///////////////////////////////////////////
// recv and send functions typedefs:
typedef void(*QNetRecvFrameFunc)(QNetFrameToRecv * frame, void ** stop_event);
typedef void(*QNetSendFrameFunc)(QNetFrameToSend * frame);

// This struct represent one network interface.
// recv_frame_func - callback that when called need to wait until packet is ready and then return with the packet.
// send_frame_func - callback that send a frame on the network interface.
// os_iface - struct of the os specific interface parameters.
struct _QNetInterface {
	struct _QNetInfterface * next;
	QNetRecvFrameFunc recv_frame_func;
	QNetSendFrameFunc send_frame_func;

	// QNet threads are not allow to take this mutex.
	// This mutex is reserved for the use of the client only when he calls to the iface management functions.
	QNetMutex * iface_mutex;
	struct QNetOsInterface * os_iface;
	enum IfaceState state;

	// iface general parameters:
	QNetThreadsPool * tpool; // threads pool for all the threads that do jobs on that interface.

							 // Raw listeners
	QNetMutex * layer2_raw_listeners_mutex;
	Layer2Listener * layer2_raw_listeners;

	// statistics:
	uint32 recv_pkts_count;
	uint32 recv_bytes_count;
	uint32 recv_pkts_drop;

	// protocols:
	enum Protocols protos;
	enum Protocols protos_up;
	EthernetProtocol eth;
	ArpProtocol arp;
};
typedef struct _QNetInterface QNetInterface;
#define IS_PROTO_ENABLE(x) ((iface->protos & (1<<x)) != 0)
#define PROTO_MARK_UP(x) (iface->protos_up |= (1<<x))
#define PROTO_MARK_DOWN(x) (iface->protos_up &= ~(1<<x))
#define IS_PROTO_UP(x) ((iface->protos_up & (1<<x)) != 0)

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////// Protocols ///////////////////////////////////////////////////////////////////////

// Every protocol needs to implement protocol start routine, that is called after the threads pool is up.
typedef QResult (*qnet_proto_start_protocol_t)(QNetStack * qstk, QNetInterface * iface);

// Every protocol needs to implement protocol end routine, that is called after all the threads in the
// threads pool are down.
typedef QResult (*qnet_proto_stop_protocol_t)(QNetStack * qstk, QNetInterface * iface);

typedef struct {
	enum Protocols proto;
	qnet_proto_start_protocol_t proto_start_func;
	qnet_proto_stop_protocol_t proto_stop_func;
} QNetProtocol;
static QNetProtocol PROTOCOLS[];
//////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////// interface interface (so funny) ////////////////////////////////////////////////////////////
EXPORT QResult qnet_init_iface(QNetStack * qstk, QNetInterface * iface, QNetRecvFrameFunc recv_frame_func, QNetSendFrameFunc send_frame_func, struct QNetOsInterface * os_iface);
EXPORT QResult qnet_iface_start(QNetStack * qstk, QNetInterface * iface);
EXPORT QResult qnet_iface_stop(QNetStack * qstk, QNetInterface * iface);
//////////////////////////////////////////////////////////////////////////////////////

////////////////// interface control helpers (up / down) ///////////////////////////////////////////////////
enum IfaceState {
	IFACE_STATE_DOWN = 0,
	IFACE_STATE_GOING_DOWN = 1,
	IFACE_STATE_GOING_UP = 2,
	IFACE_STATE_UP = 3,
	IFACE_STATE_NUM_OF_STATES = 4
};

// g_iface_state_transitions[x] = y, means that you can move to state x just from state y.
static g_iface_state_transitions[IFACE_STATE_NUM_OF_STATES];
/////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////// Internal functions and structs ////////////////////////////////////////////////////////////////
typedef struct {
	QNetStack * qstk;
	QNetInterface * iface;
} QNetFrameListenerFuncParams;
// This is the thread that listen to incoming packets and start the handaling process
void qnet_frame_listener_thread(QNetFrameListenerFuncParams * param);

// this is a helper function to change the state of a interface.
BOOL qnet_iface_change_state(QNetInterface * iface, enum IfaceState new_state);

// This function handle single incoming frame. called by the listener thread.
QResult qnet_handle_frame(QNetStack * qstk, QNetInterface * iface, QNetFrameToRecv * frame);


////////////////////////// Raw packets listenres ////////////////////////////////////////////////////////////
struct _Layer2Listener {
	struct _Layer2Listener * next;
	QResult(*handle_frame)();
};
typedef struct _Layer2Listener Layer2Listener;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //__QNET_IFACE_H__
