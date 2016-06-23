#ifndef QBJECT_H
#define QBJECT_H
#include "../Common/Qube.h"
typedef uint32 QNodeType;
typedef uint32 ACCESS;
#define ACCESS_READ (1)
#define ACCESS_WRITE (2)
//Qbject
typedef void* QbjectContent;

struct QNode_;

typedef struct {
	struct QNode_* associated_qnode;
	uint8 content[0];
} Qbject;

typedef void* QHandle;

//QbjectNode

typedef QHandle (*CreateQbjectFunction)(void* qnode_context, char* path, ACCESS access, uint32 flags);
typedef QResult(*QbjectReadFunction)(QHandle qbject, uint8* buffer, uint64 position, uint64 num_of_bytes_to_read, uint64* res_num_read);
typedef QResult(*QbjectWriteFunction)(QHandle qbject, uint8* buffer, uint64 position, uint64 num_of_bytes_to_write, uint64* res_num_written);
typedef void* (*QbjectGetPropertyFunction)(Qbject* qbject, uint32 id);
#define MAX_QNODE_NAME_LEN (0x20)
typedef struct QNode_{
	QNodeType type;
	CreateQbjectFunction create_qbject;
	QbjectReadFunction read;
	QbjectWriteFunction write;
	QbjectGetPropertyFunction get_property;
	struct QNode_* left_child;
	struct QNode_* rigth_sibling;
	void* qnode_context;
	char name[MAX_QNODE_NAME_LEN];
} QNode;

typedef struct {
	void* qnode_context;
	CreateQbjectFunction create_qbject;
	QbjectReadFunction read;
	QbjectWriteFunction write;
	QbjectGetPropertyFunction get_property;
}
QNodeAttributes;

EXPORT QResult create_qnode(char * path);

EXPORT QHandle create_qbject(char* path, ACCESS access);

EXPORT QResult read_qbject(QHandle qhandle, uint8* buffer, uint64 position, uint64 num_of_bytes_to_read, uint64* res_num_read);

EXPORT QResult write_qbject(QHandle qhandle, uint8* buffer, uint64 position, uint64 num_of_bytes_to_read, uint64* res_num_write);

EXPORT QHandle allocate_qbject(uint32 content_size);

EXPORT QResult set_qnode_attributes(char* path, QNodeAttributes* attr);

EXPORT QbjectContent get_qbject_content(QHandle handle) {
	return (void*)(&((Qbject*)handle)->content);
}
#define QNODE_TYPE_ROOT (0)
#define QNODE_TYPE_GENERIC (1)
#define CREATE_QBJECT_FLAGS_SECOND_CHANCE (1)
#endif