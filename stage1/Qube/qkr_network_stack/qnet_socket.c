#include "qnet_socket.h"

QNetSocket * socket(enum QNetSocketDomain domain, enum QNetSocketType type, enum QNetSocketProtocol protocol) {
	
	switch (domain) {
	case AF_PACKET:
		switch (type) {
		case SOCK_RAW:
			switch (protocol) {
			case ETH_P_ALL:
				return NULL;
			default:
				return NULL;
			}
		default:
			return NULL;
		}
	default:
		return NULL;
	}
}