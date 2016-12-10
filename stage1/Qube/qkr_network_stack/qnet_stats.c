#include "qnet_stats.h"

void qnet_stats_pkt_arrive(QNetStack * qstk, QNetInterface * iface, QnetFrameToRecv * frame) {
	iface->recv_pkts_count++;
	iface->recv_bytes_count += get_packet_size(frame->pkt);
	return;
}
