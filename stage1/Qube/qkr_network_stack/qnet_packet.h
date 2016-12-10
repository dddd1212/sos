#ifndef __QNET_PACKET__H__
#define __QNET_PACKET__H__
#include "qnet_os.h"
struct _Packet {
struct _Packet * next;
uint32 num_of_chunks;

uint8 * buf; // The memory
uint32 buf_actual_size; // the allocated size.
uint32 but_use_size; // The size that actually in use.

// These fields are defined only for the first chunk:
BOOL is_packet_memory; // Is the memory belong to the special packets allocator.
uint32 packet_total_size;
uint32 packet_num_of_chunks;

};
typedef struct _Packet Packet;

uint32 qnet_pkt_get_size(Packet * pkt);

// consume some bytes. For example, if we process the Ethernet layer, and want to move to the next layer,
// We can consume 14 bytes, and the packet will looks like a packet without a ethernet layer.
QResult qnet_pkt_consume_bytes(Packet * pkt, uint32 size);

// Add bytes in the start of the packet. This function can be use to add aditional headers to packet.
QResult qnet_pkt_add_bytes_to_start(Packet * pkt, uint32 size);

// Add bytes in the end of the packet.
QResult qnet_pkt_add_bytes_to_end(Packet * pkt, uint32 size);

// After success calling to this function, pkt contains for sure at least size bytes in the same buffer.
// That means that you may use a pointer to the data and to assume that you got at least size bytes on the ptr.
QResult qnet_pkt_same_buf_arrange(Packet * pkt, uint32 size);

uint8 * qnet_pkt_get_ptr(Packet * pkt, uint32 offset);
#endif // __QNET_PACKET__H__