#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H
#include "../Common/Qube.h"
#include "../MemoryManager/memory_manager.h"
#include "../scheduler/scheduler.h"
#include "../Common/spin_lock.h"
#include "../QbjectManager/Qbject.h"
#include "../qkr_sync/sync.h"
typedef void* NetworkInterfaceManagerContext;
typedef void* NetworkQueueManagerContext;
typedef void* NetworkInterfaceDriverContext;
typedef void* NetworkQueueDriverContext;

typedef struct NetworkData_ NetworkData;

typedef QResult(*ReleaseNetworkDataFunc)(NetworkData* network_data);

typedef struct NetworkBuffer_ {
	struct NetworkBuffer* next;
	uint8* buffer;
	uint64 buffer_size;
} NetworkBuffer;

typedef enum {
	ETH_RAW,
	IPV4,
	IPV6,
	IPV4_TCP,
	IPV4_UDP,
	IPV6_TCP,
	IPV6_UDP
}NetworkDataType;

typedef struct EthNetworkData_ {
	NetworkBuffer* network_buffer;
	uint64 eth_data_offset;
} EthNetworkData;

struct NetworkData_ {
	NetworkData* next_network_data; // used to concatenate NetworkData structures (in some queue for example)
	NetworkDataType data_type;
	ReleaseNetworkDataFunc free_network_data;
	NetworkBuffer* first_buffer;
	union {
		EthNetworkData eth_data;
	};
};

typedef QResult(*InitializeNetworkInterfaceFunc)(NetworkInterfaceManagerContext network_interface_manager_context, NetworkInterfaceDriverContext network_interface_driver_context);
typedef QResult(*GetNetworkDataFunc)(NetworkQueueDriverContext queue_driver_context, NetworkData* out);
typedef BOOL(*IsQueueEmptyFunc)(NetworkQueueDriverContext queue_driver_context);

typedef struct NetworkQueueManagerContextI_ {
	NetworkQueueDriverContext driver_context;
	struct NetworkInterfaceManagerContextI_* related_interface;
	IsQueueEmptyFunc is_queue_empty;
	GetNetworkDataFunc get_network_data;
	BOOL emptying_queue;
	SpinLock emptying_queue_lock;
} NetworkQueueManagerContextI;

typedef struct NetworkInterfaceManagerContextI_ {
	NetworkInterfaceDriverContext driver_context;
	BOOL qbject_created;
	uint32 num_of_queues;
	NetworkQueueManagerContext queue_list[5]; // TODO: for now, this is the maximum number of queues, but it should not work this way.
	Event data_available_event;
	NetworkData* global_queue_head;

} NetworkInterfaceManagerContextI;


typedef struct NetworkInterfaceRegisterData_ {
	NetworkInterfaceDriverContext network_interface_driver_context;
	InitializeNetworkInterfaceFunc initialize;
} NetworkInterfaceRegisterData;

typedef struct NetworkQueueRegisterData_ {
	NetworkQueueDriverContext queue_driver_context;
	GetNetworkDataFunc get_network_data;
	IsQueueEmptyFunc is_queue_empty;
} NetworkQueueRegisterData;


EXPORT QResult register_interface(NetworkInterfaceRegisterData* interface_data);
EXPORT QResult register_queue(NetworkInterfaceManagerContext nm_interface_context, NetworkQueueRegisterData* queue_register_data, NetworkQueueManagerContext** nm_queue_context);
EXPORT QResult notify_queue_is_not_empty(NetworkQueueManagerContext nm_context);

#endif
