#ifndef HD_H
#define HD_H

#include "Qube.h"
#include "intrinsics.h"
int read_sectors(uint32 LBA, uint8 numOfSectors, void *out_buf);


#endif