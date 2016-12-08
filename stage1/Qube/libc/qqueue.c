#include "qqueue.h"
#include "../MemoryManager/heap.h"
#include "string.h"



QQueue * create_qqueue(uint32 num_of_elements, uint32 element_size) {
	QQueue * ret = (QQueue*)kheap_alloc(sizeof(QQueue) + num_of_elements * element_size);
	if (ret == NULL) return NULL;
	ret->element_size = element_size;
	ret->queue_data = (uint8*)(ret + 1);
	ret->queue_data_end = ret->queue_data + num_of_elements * element_size;
	ret->head_ptr = ret->queue_data;
	ret->tail_ptr = ret->queue_data;
	ret->queue_cur_size = 0;
	ret->queue_max_size = num_of_elements;
	return ret;
}

void destroy_qqueue(QQueue * qq) {
	kheap_free((void*)qq);
	return;
}
BOOL enqqueue(QQueue * qq, void * data) {
	if (qq->queue_cur_size == qq->queue_max_size) return FALSE;
	memcpy(qq->head_ptr, data, qq->element_size);
	qq->head_ptr += qq->element_size;
	if (qq->head_ptr == qq->queue_data_end) {
		qq->head_ptr = qq->queue_data;
	}
	qq->queue_cur_size++;
	return TRUE;
}

BOOL deqqueue(QQueue * qq, void * out) {
	if (qq->queue_cur_size == 0) return FALSE;
	memcpy(out, qq->tail_ptr, qq->element_size);
	qq->tail_ptr += qq->element_size;
	if (qq->tail_ptr == qq->queue_data_end) {
		qq->tail_ptr = qq->queue_data;
	}
	qq->queue_cur_size--;
	return TRUE;
}

uint32 qqueue_cur_size(QQueue * qq) {
	return qq->queue_cur_size;
}