#ifndef __QNETWORK_STACK__H__
#define __QNETWORK_STACK__H__
#include "qnet_os.h"
#include "qnet_defs.h"


// This struct is the general context of the network stack. It contains everything.
typedef struct {
	QNetMutex * ifaces_mutex;
	QNetInterface * ifaces;
} QNetStack;

EXPORT QResult qnet_create_stack(QNetStack * qstk);
EXPORT QResult qnet_register_interface(QNetStack * qstk, QNetInterface * iface);
EXPORT QResult qnet_deregister_interface(QNetStack * qstk, QNetInterface * iface);






#endif // __QNETWORK_STACK__H__