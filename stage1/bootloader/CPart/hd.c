#include "Qube.h"
#include "intrinsics.h"
int read_sectors(uint32 LBA, uint8 numOfSectors, void *out_buf){
	uint8 read_status;
	__out8(0x1f6, (LBA>>24)|0b11100000) ;
	__out8(0x1f2, numOfSectors);
	__out8(0x1f3, (int8)LBA);
	__out8(0x1f4, (int8)(LBA>>8));
	__out8(0x1f5, (int8)(LBA>>16));

	__out8(0x1f7, 0x20); //command port. read with retry
	do{
		read_status = __in8(0x1f7);
	}while (!(read_status&8));
	__insw(0x1f0, numOfSectors*0x100, out_buf);
	return 0; //success
}