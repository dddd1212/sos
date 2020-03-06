#include "qnet_stats.h"

void qnet_stats_pkt_arrive(QNetStack * qstk, QNetInterface * iface, QNetFrameToRecv * frame) {
	iface->recv_pkts_count++;
	iface->recv_bytes_count += qnet_pkt_get_size(frame->pkt);
	return;
}
void qnet_stats_pkt_drop(QNetStack * qstk, QNetInterface * iface, QNetFrameToRecv * frame) {
	iface->recv_pkts_drop++;
}

void qnet_stats_reset_counters(QNetStack * qstk, QNetInterface * iface) {
	iface->recv_pkts_count = 0;
	iface->recv_bytes_count = 0;
	iface->recv_pkts_drop = 0;
}