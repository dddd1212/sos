#ifndef __QNET_PACKET__H__
#define __QNET_PACKET__H__
#include "qnet_os.h"
struct _PacketChunk {
	struct _PacketChunk * next;
	struct _PacketChunk * prev;
	
	// Real data parameters:
	uint8 * buf; // The memory
	uint32 buf_memory_size; // The allocated size.
	BOOL is_packet_memory; // Is the memory belong to the special packets allocator.	


	// Data use parameters:
	uint32 buf_start; // The start offset of the buffer.
	uint32 buf_size; // The size that actually in use.
	// The data outside of these limits consider undefined and couldn't be referenced!
};
typedef struct _PacketChunk PacketChunk;

struct _Packet {
struct _PacketChunk * first;
struct _PacketChunk * last;

uint32 packet_memory_size;
uint32 packet_size;

// pointer to the current pointer in the packet.
struct _PacketChunk * cur; // the current chunk.
uint32 cur_chunk_offset; // the relative offset from the start of the chunk.

uint32 ptr_offset; // The absulote offset of the ptr from the start of the packet.

};
typedef struct _Packet Packet;

uint32 qnet_pkt_get_size(Packet * pkt);

// move the cur_ptr some bytes. For example, if we process the Ethernet layer, and want to move to the next layer,
// We can consume 14 bytes, and the packet will looks like a packet without a ethernet layer. (But the ethernet layer is still valid).
QResult qnet_pkt_consume_bytes(Packet * pkt, uint32 size);

// Add bytes to the start of the packet. This function can be use to add aditional headers to packet.
QResult qnet_pkt_add_bytes_to_start(Packet * pkt, uint32 size);

// Add bytes to the end of the packet.
QResult qnet_pkt_add_bytes_to_end(Packet * pkt, uint32 size);

// After success calling to this function, pkt contains for sure at least size bytes in the same buffer.
// That means that you may use a pointer to the data and to assume that you got at least size bytes on the ptr.
QResult qnet_pkt_same_buf_arrange(Packet * pkt, uint32 size);

uint8 * qnet_pkt_get_ptr(Packet * pkt, uint32 offset);

QResult qnet_pkt_free_packet_chunk(PacketChunk * cur);
// Free the whole packet structs. after this call, the pointer is invalid couldn't be used.
QResult qnet_pkt_free_packet(Packet * pkt);
#endif // __QNET_PACKET__H__