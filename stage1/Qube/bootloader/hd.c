#include "../Common/Qube.h"
#include "hd.h"
#include "fat32.h"

QResult init_hd(BootLoaderAllocator *allocator, hdDesc *desc){
	desc->type = FAT32;
	return init_FAT32(allocator, 0, &desc->desc.fat32desc);
}

int32 get_file_size(hdDesc *desc, char *filename){
	if (desc->type == FAT32){
		return fat32_get_file_size(&desc->desc.fat32desc,filename);
	}
	return -1;
}
QResult read_file(hdDesc *desc, char *filename, int8 *out_buf){
	if (desc->type == FAT32) {
		return fat32_read_file(&desc->desc.fat32desc, filename, out_buf);
	}
	return QFail;
}
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