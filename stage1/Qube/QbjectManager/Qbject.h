#ifndef QBJECT_H
#define QBJECT_H
#include "../Common/Qube.h"
typedef uint32 QNodeType;
typedef uint32 ACCESS;
//Qbject
typedef void* QbjectContent;

typedef struct QNode_ QNode;

typedef struct {
	QNode* associated_qnode;
	uint8 content[0];
} Qbject;

typedef void* QHandle;

//QbjectNode

typedef Qbject* (*CreateQbjectFunction)(char* path, ACCESS access);
typedef QResult(*QbjectReadFunction)(Qbject* qbject, uint8* buffer, uint64 num_of_bytes_to_read, uint64* res_num_read);
typedef QResult(*QbjectWriteFunction)(Qbject* qbject, uint8* buffer, uint64 num_of_bytes_to_write, uint64* res_num_written);

struct QNode_{
	QNodeType type;
	CreateQbjectFunction create_qbject;
	QbjectReadFunction read;
	QbjectWriteFunction write;
	QNode* left_child;
	QNode* rigth_sibling
};

QHandle create_qbject(char* path, ACCESS access);

QHandle allocate_qbject(uint32 content_size);

QbjectContent inline get_qbject_content(QHandle handle) {
	return (void*)(&((Qbject*)handle)->content);
}
#define QNODE_TYPE_ROOT (0)
#endif