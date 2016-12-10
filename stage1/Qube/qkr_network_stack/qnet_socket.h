#ifndef __QNET_SOCKET_H__
#define __QNET_SOCKET_H__
#include "qnetstack.h"

enum QNetSocketDomain {
	AF_PACKET = 0,
};
enum QNetSocketType {
	SOCK_RAW = 0,
};

enum QNetSocketProtocol {
	ETH_P_ALL = 0,
};
typedef struct {
	uint32 todo;
} QNetSocket;

QNetSocket * socket(enum QNetSocketDomain domain, enum QNetSocketType type, enum QNetSocketProtocol protocol);
QResult setsockopt(QNetSocket * sock, uint32 level, uint32 optname, const void *optval, uint32 optlen);
uint32 send(QNetSocket * sock, const uint8 *buf, uint32 len, uint32 flags);
uint32 recv(QNetSocket * sock, const uint8 *buf, uint32 len, uint32 flags);
QResult closesocket(QNetSocket * sock);

#endif // __QNET_SOCKET_H__

