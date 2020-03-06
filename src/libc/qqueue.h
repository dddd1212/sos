#ifndef __Q_QUEUE_H_
#define __Q_QUEUE_H_
#include "../Common/Qube.h"


// TODO: for now, this queue is NOT THREAD SAFE.
typedef struct {
	uint8 * queue_data;
	uint8 * queue_data_end;
	uint8 * head_ptr;
	uint8 * tail_ptr;
	uint32 element_size;
	uint32 queue_cur_size;
	uint32 queue_max_size;
} QQueue;

EXPORT QQueue * create_qqueue(uint32 num_of_elements, uint32 element_size);
EXPORT void destroy_qqueue(QQueue * qq);
EXPORT BOOL enqqueue(QQueue * qq, void * data);
EXPORT BOOL deqqueue(QQueue * qq, void * out);
EXPORT uint32 qqueue_cur_size(QQueue * qq);


#endif // __Q_QUEUE_H_
