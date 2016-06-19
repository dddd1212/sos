#ifndef QBJECT_H
#define QBJECT_H
#include "../Common/Qube.h"
typedef uint32 QNodeType;
typedef uint32 ACCESS;
//Qbject
typedef void* QbjectContent;

struct QNode_;

typedef struct {
	struct QNode_* associated_qnode;
	uint8 content[0];
} Qbject;

typedef void* QHandle;

//QbjectNode

typedef Qbject* (*CreateQbjectFunction)(char* path, ACCESS access, uint32 flags);
typedef QResult(*QbjectReadFunction)(Qbject* qbject, uint8* buffer, uint64 num_of_bytes_to_read, uint64* res_num_read);
typedef QResult(*QbjectWriteFunction)(Qbject* qbject, uint8* buffer, uint64 num_of_bytes_to_write, uint64* res_num_written);
#define MAX_QNODE_NAME_LEN (0x20)
typedef struct QNode_{
	QNodeType type;
	CreateQbjectFunction create_qbject;
	QbjectReadFunction read;
	QbjectWriteFunction write;
	struct QNode_* left_child;
	struct QNode_* rigth_sibling;
	char name[MAX_QNODE_NAME_LEN];
} QNode;

EXPORT QResult create_qnode(char * path);

EXPORT QHandle create_qbject(char* path, ACCESS access);

EXPORT QHandle allocate_qbject(uint32 content_size);

QbjectContent inline get_qbject_content(QHandle handle) {
	return (void*)(&((Qbject*)handle)->content);
}
#define QNODE_TYPE_ROOT (0)
#define QNODE_TYPE_GENERIC (1)
#define CREATE_QBJECT_FLAGS_SECOND_CHANCE (1)
#endif