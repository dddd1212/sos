#ifndef HD_H
#define HD_H

#include "../Common/Qube.h"
#include "fat32.h"

typedef enum {
	FAT32
} hdType;

typedef struct{
	int32 type;
	union {
		FAT32Desc fat32desc;
	} desc;
} hdDesc;
int32 init_hd(BootLoaderAllocator *allocator, hdDesc *desc);
int32 get_file_size(hdDesc *desc, char *filename);
int32 read_file(hdDesc *desc, char *filename, int8 *out_buf);
int read_sectors(uint32 LBA, uint8 numOfSectors, void *out_buf);


#endif