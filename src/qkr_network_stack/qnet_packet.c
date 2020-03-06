#include "qnet_packet.h"


uint32 qnet_pkt_get_size(Packet * pkt) {
	return pkt->packet_size; // return the packet 
}

// consume some bytes. For example, if we process the Ethernet layer, and want to move to the next layer,
// We can consume 14 bytes, and the packet will looks like a packet without a ethernet layer.
QResult qnet_pkt_consume_bytes(Packet * pkt, uint32 size);

QResult qnet_pkt_go_back_bytes(Packet * pkt, uint32 size);

// Add bytes in the start of the packet. This function can be use to add aditional headers to packet.
QResult qnet_pkt_add_bytes_to_start(Packet * pkt, uint32 size);

// Add bytes in the end of the packet.
QResult qnet_pkt_add_bytes_to_end(Packet * pkt, uint32 size);


QResult qnet_pkt_same_buf_arrange(Packet * pkt, uint32 size) {
	uint32 bytes_in_row = pkt->cur->buf_size - pkt->cur_chunk_offset;
	if (bytes_in_row >= size) return QSuccess;
	if (pkt->packet_size - pkt->ptr_offset < size) { // Not enough data in the packet!
		return QFail;
	}
	// bytes from the cur_chunk_offset to the end of the allocation:
	uint32 bytes_available_in_our_chunk = pkt->cur->buf_memory_size - pkt->cur->buf_start - pkt->cur_chunk_offset;
	if (bytes_available_in_our_chunk < size) { // need to realloc:
		uint8 * buf = NULL;
		// The new chunk buffer size will be the size wanted + the offset from the start of the chunk.
		uint32 size_to_alloc = size + pkt->cur_chunk_offset;
		
		if (pkt->cur->is_packet_memory) { // in that case, copy the buffer to regular memory:
			buf = qnet_malloc(size_to_alloc);
			if (buf == NULL) return QFail;
			qnet_memcpy(buf, pkt->cur->buf + pkt->cur->buf_start, pkt->cur->buf_size);
			qnet_free_packet(pkt->cur->buf);
			pkt->cur->buf = buf;
			pkt->cur->buf_start = 0;
			pkt->cur->buf_memory_size = size_to_alloc;
			pkt->cur->is_packet_memory = FALSE;
			pkt->cur->buf_size = size_to_alloc;
		} else {
			pkt->cur->buf_size = size_to_alloc;
			size_to_alloc += pkt->cur->buf_start; // there might be unused data before the address where buf actually starts. in that case we just ignore this data and leave it alone.
			pkt->cur->buf = qnet_realloc(pkt->cur->buf, size_to_alloc);
			if (pkt->cur->buf == NULL) return QFail;
			pkt->cur->buf_memory_size = size_to_alloc;

		}
	}
	// Here we have this state:
	// The cur chunk is on the regular memory. our buffer could start buf_start bytes from the star to the buf pointer.
	// We have at least 'size' bytes after the start of the buffer.
	// The data in the buf is the original data
	// ptr will point right after the last byte that already in the buffer.
	uint8 * ptr = pkt->cur->buf + pkt->cur->buf_start + pkt->cur_chunk_offset + bytes_in_row;
	PacketChunk * next = pkt->cur->next;
	PacketChunk * next_next;
	while (bytes_in_row < size) {
		next_next = next->next;
		qnet_memcpy(ptr, next->buf + next->buf_start, next->buf_size);
		bytes_in_row += next->buf_size;
		qnet_pkt_free_packet_chunk(next);
		next = next_next;
	}
	return QSuccess;
}
QResult qnet_pkt_free_packet_chunk(PacketChunk * cur) {
	if (cur->is_packet_memory) {
		qnet_free_packet(cur->buf);
	}
	else {
		qnet_free(cur->buf);
	}
	qnet_free((uint8*)cur);
	return QSuccess;
}
QResult qnet_pkt_free_packet(Packet * pkt) {
	PacketChunk *next, * cur = pkt->first;
	for (cur = pkt->first; cur; cur = next) {
		next = cur->next;
		qnet_pkt_free_packet_chunk(cur);
	}
	qnet_free((uint8*)pkt);
	return QSuccess;
}
