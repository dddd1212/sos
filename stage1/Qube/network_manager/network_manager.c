#include "network_manager.h"
#include "../libc/string.h"
BOOL g_interface_registered;
QResult qkr_main(KernelGlobalData* kgd) {
	g_interface_registered = FALSE;
	return QSuccess;
}

QResult empty_network_queue(NetworkQueueManagerContextI* nm_queue_context) {
	// This is a system task so scheduling is off.
	NetworkData* network_data;
	while (1) {
		// first, mark that there are no new data (if there were, we pull it now)
		{
			QueueWorkerStatus new_status = { 0 };
			new_status.new_data = 0;
			new_status.worker_run = 1;
			interlocked_xchg(&nm_queue_context->queue_worker_status.value, new_status.value);
		}

		// second, check if the queue is empty and break if it is
		if (nm_queue_context->is_queue_empty(nm_queue_context->driver_context)) {
			QueueWorkerStatus old_status = { 0 };
			QueueWorkerStatus new_status = { 0 };
			new_status.worker_run = 0;
			new_status.new_data = 0;
			old_status.worker_run = 1;
			old_status.new_data = 0;
			old_status.value = interlocked_cmpxchg(&nm_queue_context->queue_worker_status.value, new_status.value, old_status.value);
			if (!old_status.new_data) {
				break;
			}
		}
		// finlly, pull some data
		if (QSuccess == nm_queue_context->get_network_data(nm_queue_context->driver_context, &network_data)) {
			spin_lock(&nm_queue_context->related_interface->global_queue_lock);
			if (NULL == nm_queue_context->related_interface->global_queue_head) {
				nm_queue_context->related_interface->global_queue_head = network_data;
				set_event(nm_queue_context->related_interface->data_available_event);
			}
			else {
				NetworkData* current = nm_queue_context->related_interface->global_queue_head;
				while (current->next_network_data) {
					current = current->next_network_data;
				}
				current->next_network_data = network_data;
			}
			spin_unlock(&nm_queue_context->related_interface->global_queue_lock);
		}
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
	QResult result = QSuccess;
	NetworkInterfaceManagerContextI* interface_manager_context = (NetworkInterfaceManagerContextI*) get_qbject_associated_qnode_context(qbject);
	wait_for_event(interface_manager_context->data_available_event);
	BOOL scheduling_enabled = is_scheduling_enabled();
	if (scheduling_enabled) {
		disable_scheduling();
	}
	spin_lock(&interface_manager_context->global_queue_lock);
	
	NetworkData* network_data = interface_manager_context->global_queue_head;
	NetworkBuffer* network_buffer = network_data->first_buffer;
	uint64 total_buffer_size = 0;
	while (network_buffer) {
		if (total_buffer_size + network_buffer->buffer_size <= num_of_bytes_to_read) {
			memcpy(buffer + total_buffer_size, network_buffer->buffer, network_buffer->buffer_size);
			total_buffer_size += network_buffer->buffer_size;
		}
		else {
			result = QBufferTooSmall;
			break;
		}
		network_buffer = network_buffer->next;
	}

	spin_unlock(&interface_manager_context->global_queue_lock);
	if (scheduling_enabled) {
		enable_scheduling();
	}
	return result;
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
	interface_manager_context->global_queue_head = NULL;
	spin_init(&interface_manager_context->global_queue_lock);
	if (QSuccess != create_event(&interface_manager_context->data_available_event)) {
		return QFail;
	}
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
	queue_context->queue_worker_status.value = 0;
	queue_context->related_interface = nm_interface_context;
	nm_interface_context->queue_list[nm_interface_context->num_of_queues] = queue_context;
	nm_interface_context->num_of_queues++;
	*nm_queue_context = queue_context;
	return QSuccess;
}

QResult notify_queue_is_not_empty(NetworkQueueManagerContext nm_queue_context_)
{
	NetworkQueueManagerContextI* nm_queue_context = (NetworkQueueManagerContextI*)nm_queue_context_;
	QueueWorkerStatus old_status = { 0 };
	QueueWorkerStatus new_status = { 0 };
	new_status.new_data = 1;
	new_status.worker_run = 1;
	
	old_status.value = interlocked_xchg(&nm_queue_context->queue_worker_status.value,new_status.value);
	
	if (!old_status.worker_run) {
		add_system_task((SystemTaskFunction)empty_network_queue, nm_queue_context);
	}
	return QSuccess;
}

QResult register_network_data_queue() {

}