#ifndef __QNET_STATS_H__
#define __QNET_STATS_H__
#include "qnetstack.h"
#include "qnet_iface.h"
void qnet_stats_pkt_arrive(QNetStack * qstk, QNetInterface * iface, QNetFrameToRecv * frame);
void qnet_stats_pkt_drop(QNetStack * qstk, QNetInterface * iface, QNetFrameToRecv * frame);
void qnet_stats_reset_counters(QNetStack * qstk, QNetInterface * iface);
#endif //__QNET_STATS_H__

