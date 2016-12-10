#include "qnet_stats.h"

void qnet_stats_pkt_arrive(QNetStack * qstk, QNetInterface * iface, QNetFrameToRecv * frame) {
	iface->recv_pkts_count++;
	iface->recv_bytes_count += qnet_pkt_get_size(frame->pkt);
	return;
}
