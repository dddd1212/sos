#include "Qube.h"
#include "hd.h"
typedef struct {
	int16 bytes_per_sector;
	int8 sectors_per_cluster;
	int16 num_of_reserved_sectors;
	int8 num_of_FATs;
	int32 num_of_hidden_sectors;
	int32 total_num_of_sectors;
	int32 num_of_sectors_in_FAT;
	int32 root_dir_cluster_num;
} FAT32Desc;

int32 initFAT32(FAT32Desc *desc, unit32 BPB_sector){
	int8 buf[0x200]; // buffer for one sector
	if((res = read_sectors(BPB_sector, 1, buf))!=0){
		return res;
	}
	desc->bytes_per_sector = *((int16*)&buf[11])
	desc->sectors_per_cluster = *((int8*)&buf[13])
	desc->num_of_reserved_sectors = *((int16*)&buf[14])
	desc->num_of_FATs = *((int8*)&buf[16])
	desc->num_of_hidden_sectors = *((int32*)&buf[28])
	desc->total_num_of_sectors = *((int32*)&buf[32])
	desc->num_of_sectors_in_FAT = *((int32*)&buf[36])
	desc->root_dir_cluster_num = *((int32*)&buf[44])
	return 0; //success.
}

int32 get_file_size(FAT32Desc *desc, char *filename){
	
}


void main(){
	FAT32Desc fat32desc;
	initFAT32(&fat32desc,0); // TODO: currently we don't support partitions, so we pass sector number 0.
}