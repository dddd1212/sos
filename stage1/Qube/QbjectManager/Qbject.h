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

//Qbject
typedef void* QbjectContent;
typedef QResult(*QbjectReadFunction)(QbjectContent, uint8* buffer, uint64 num_of_bytes_to_read, uint64* res_num_read);
typedef QResult(*QbjectWriteFunction)(QbjectContent, uint8* buffer, uint64 num_of_bytes_to_write, uint64* res_num_written);

//QbjectContainer
typedef Qbject* (*QbjectOpenFunction)(QbjectContent, uint8* buffer, uint64 num_of_bytes_to_write, uint64* res_num_written);


typedef struct {
	QbjectType type;
	QbjectContent content;
	QbjectReadFunction read_func;
	QbjectWriteFunction write_func;
} Qbject;

typedef struct {
	// same as Qbject
	QbjectType type;
	QbjectContent content;
	QbjectReadFunction read_func;
	QbjectWriteFunction write_func;
	
	// additional fields for containers
	QbjectOpenFunction open;
} QbjectContainer;

#endif