#ifndef FAT32_H
#define FAT32_H

#include "mem.h"
typedef struct{
	int16 bytes_per_sector;
	int8 sectors_per_cluster;
	int16 num_of_reserved_sectors;
	int8 num_of_FATs;
	int32 num_of_hidden_sectors;
	int32 total_num_of_sectors;
	int32 num_of_sectors_in_FAT;
	int32 root_dir_cluster_num;
	
	int32 first_data_sector;
	int32 *FAT;
	int8 *cluster_buf;

	Allocator *allocator;
} FAT32Desc;

int32 fat32_get_file_size(FAT32Desc *fat32_desc, char *filename);
int32 init_FAT32(Allocator *allocator, uint32 BPB_sector, FAT32Desc *fat32_desc);

#endif