#include "qnet_packet.h"

uint32 qnet_pkt_get_size(Packet * pkt) {
	return pkt->packet_total_size;
}

// consume some bytes. For example, if we process the Ethernet layer, and want to move to the next layer,
// We can consume 14 bytes, and the packet will looks like a packet without a ethernet layer.
QResult qnet_pkt_consume_bytes(Packet * pkt, uint32 size);

// Add bytes in the start of the packet. This function can be use to add aditional headers to packet.
QResult qnet_pkt_add_bytes_to_start(Packet * pkt, uint32 size);

// Add bytes in the end of the packet.
QResult qnet_pkt_add_bytes_to_end(Packet * pkt, uint32 size);


