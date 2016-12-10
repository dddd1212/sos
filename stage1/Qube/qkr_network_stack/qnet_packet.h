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

uint32 get_packet_size(Packet * pkt) {
	return pkt->packet_total_size;
}

#endif // __QNET_PACKET__H__