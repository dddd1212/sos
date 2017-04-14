#include "network_manager.h"
BOOL g_interface_registered;
QResult qkr_main(KernelGlobalData* kgd) {
	g_interface_registered = FALSE;
	return QSuccess;
}

QResult empty_network_queue(NetworkQueueManagerContextI* nm_queue_context) {
	NetworkData network_data;
	while (nm_queue_context->is_queue_empty(nm_queue_context->driver_context)) {
		nm_queue_context->get_network_data(nm_queue_context->driver_context, &network_data);
	}
}

QHandle create_network_qbject(void* qnode_context, char* path, ACCESS access, uint32 flags) {
	NetworkInterfaceManagerContextI* interface_manager_context = (NetworkInterfaceManagerContextI*)qnode_context;
	if (interface_manager_context->qbject_created) {
		return NULL;
	}
	interface_manager_context->qbject_created = TRUE;
	QHandle qhandle = allocate_qbject(0);
	return qhandle;
}

QResult read_network_qbject(QHandle qbject, uint8* buffer, uint64 position, uint64 num_of_bytes_to_read, uint64* res_num_read) {
	NetworkInterfaceManagerContextI* interface_manager_context = (NetworkInterfaceManagerContextI*) get_qbject_associated_qnode_context(qbject);
	wait_for_event(interface_manager_context->data_available_event);
	NetworkData* network_data;

	dequeue_network_data(interface_manager_context, &network_data);
	NetworkBuffer* network_buffer = network_data->first_buffer;
	uint64 total_buffer_size;
	while (network_buffer) {
		total_buffer_size += network_buffer->buffer_size;
		network_buffer = network_buffer->next;
	}

	network_data->first_buffer->buffer;
	return QFail;
}

QResult register_interface(NetworkInterfaceRegisterData* interface_data) {
	if (g_interface_registered == TRUE) {
		return QFail;
	}
	g_interface_registered = TRUE;
	NetworkInterfaceManagerContextI* interface_manager_context = kheap_alloc(sizeof(NetworkInterfaceManagerContext));
	interface_manager_context->driver_context = interface_data->network_interface_driver_context;
	interface_manager_context->num_of_queues = 0;
	interface_manager_context->qbject_created = FALSE;
	create_event(interface_manager_context->data_available_event);
	interface_data->initialize(interface_manager_context, interface_data->network_interface_driver_context);
	create_qnode("/Device/Network/Interfaces/Interface0");
	QNodeAttributes qnode_attrs;
	qnode_attrs.create_qbject = create_network_qbject;
	qnode_attrs.get_property = NULL;
	qnode_attrs.qnode_context= interface_manager_context;
	qnode_attrs.read = read_network_qbject;
	qnode_attrs.write = NULL;

	set_qnode_attributes("/Device/Network/Interfaces/Interface0", &qnode_attrs);
}

QResult register_queue(NetworkInterfaceManagerContext nm_interface_context_, NetworkQueueRegisterData * queue_register_data, NetworkQueueManagerContext ** nm_queue_context_)
{
	NetworkInterfaceManagerContextI* nm_interface_context = (NetworkInterfaceManagerContextI*)nm_interface_context_;
	NetworkQueueManagerContextI** nm_queue_context = (NetworkQueueManagerContextI**)nm_queue_context_;
	if (nm_interface_context->num_of_queues >= 5) {
		return QFail;
	}
	NetworkQueueManagerContextI* queue_context = kheap_alloc(sizeof(NetworkQueueManagerContextI));
	queue_context->driver_context = queue_register_data->queue_driver_context;
	queue_context->get_network_data = queue_register_data->get_network_data;
	queue_context->is_queue_empty = queue_register_data->is_queue_empty;
	queue_context->emptying_queue = FALSE;
	spin_init(&queue_context->emptying_queue_lock);
	queue_context->related_interface = nm_interface_context;
	nm_interface_context->queue_list[nm_interface_context->num_of_queues] = queue_context;
	nm_interface_context->num_of_queues++;
	*nm_queue_context = queue_context;
	return QSuccess;
}

QResult notify_queue_is_not_empty(NetworkQueueManagerContext nm_queue_context_)
{
	NetworkQueueManagerContextI* nm_queue_context = (NetworkQueueManagerContextI*)nm_queue_context_;
	BOOL emptying_queue;
	spin_lock(&nm_queue_context->emptying_queue_lock);
	emptying_queue = nm_queue_context->emptying_queue;
	spin_unlock(&nm_queue_context->emptying_queue_lock);
	if (!emptying_queue) {
		add_system_task((SystemTaskFunction)empty_network_queue, nm_queue_context);
	}
	return QSuccess;
}

QResult register_network_data_queue() {

}