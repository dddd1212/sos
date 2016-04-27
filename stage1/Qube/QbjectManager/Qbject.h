#ifndef QBJECT_H
#define QBJECT_H
#include "../Common/inc/Qube.h"
typedef union {
	uint32 value;
	struct {
		uint32 id : 31;
		uint32 isContainer : 1;
	};
} QbjectType;

typedef QResult(*QbjectReadFunction)(QobjectContent, uint8* buffer, uint64 num_of_bytes_to_read, uint64* res_num_read);
typedef QResult(*QbjectWriteFunction)(QobjectContent, uint8* buffer, uint64 num_of_bytes_to_write, uint64* res_num_written);
typedef void* QobjectContent;

typedef struct {
	QbjectType type;
	QbjectReadFunction read_func;
	QbjectWriteFunction write_func;
	QobjectContent content;
} Qbject;

#endif