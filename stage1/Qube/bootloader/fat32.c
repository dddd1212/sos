#include "Qube.h"
#include "hd.h"
#include "mem.h"

typedef struct{
	char name[11];
	int8 attr;
	int8 reserved1;
	int8 crtTimeTenth;
	int16 crtTime;
	int16 crtDate;
	int16 lstAccDate;
	int16 fstClusHi;
	int16 wrtTime;
	int16 wrtDate;
	int16 fstClusLo;
	int32 fileSize;
} __attribute__((packed)) DirectoryEntry;

int32 init_FAT32(Allocator *allocator, uint32 BPB_sector, FAT32Desc *fat32_desc){
	int8 buf[0x200]; // buffer for one sector
	int32 num_of_sectors;
	int32 res;
	if((res = read_sectors(BPB_sector, 1, buf))!=0){
		return res;
	}
	fat32_desc->bytes_per_sector = *((int16*)&buf[11]);
	fat32_desc->sectors_per_cluster = *((int8*)&buf[13]);
	fat32_desc->num_of_reserved_sectors = *((int16*)&buf[14]);
	fat32_desc->num_of_FATs = *((int8*)&buf[16]);
	fat32_desc->num_of_hidden_sectors = *((int32*)&buf[28]);
	fat32_desc->total_num_of_sectors = *((int32*)&buf[32]);
	fat32_desc->num_of_sectors_in_FAT = *((int32*)&buf[36]);
	fat32_desc->root_dir_cluster_num = *((int32*)&buf[44]);

	fat32_desc->first_data_sector = fat32_desc->num_of_reserved_sectors+fat32_desc->num_of_sectors_in_FAT*fat32_desc->num_of_FATs;
	fat32_desc->allocator = allocator;
	num_of_sectors = fat32_desc->total_num_of_sectors-fat32_desc->first_data_sector;
	fat32_desc->FAT = (int32*)mem_alloc(fat32_desc->allocator, num_of_sectors*sizeof(int32)/fat32_desc->sectors_per_cluster, TRUE);
	fat32_desc->cluster_buf = (int8*)mem_alloc(fat32_desc->allocator, fat32_desc->sectors_per_cluster*fat32_desc->bytes_per_sector, TRUE);
	if (fat32_desc->FAT == 0){
		return -1;
	}
	read_sectors(fat32_desc->num_of_hidden_sectors+fat32_desc->num_of_reserved_sectors, num_of_sectors, fat32_desc->FAT);
	return 0; //success.
}

static int32 get_next_cluster(FAT32Desc *fat32_desc, int32 cluster){
	return fat32_desc->FAT[cluster-2]; // 2 is the first data cluster.
}

static inline int32 name_to_cname(char *name, int32 namelen, char *cname){
	int32 i,j;
	for (i=0; i<8 && i<namelen;i++){
		if (name[i]=='.'){
			break;
		}
		cname[i] = name[i];
	}

	for(j=i;j<8;j++){
		cname[j]=' ';
	}

	if (name[i]=='.'){
		i++;
	}
	
	for (j=0; j<3 && (i+j)<namelen;j++){
		cname[8+j] = name[i+j];
	}

	for(;j<3;j++){
		cname[j]=' ';
	}
}


static int32 get_child_directory_entry(FAT32Desc *fat32_desc, DirectoryEntry *parent_entry, char *name, int32 namelen, DirectoryEntry *res_entry){
	char cname[11];
	int32 cur_cluster, next_cluster;
	int32 directories_in_cluster_mask;
	DirectoryEntry *directory_entry;
	int32 i,j;

	name_to_cname(name, namelen, cname);
	cur_cluster = 0;
	next_cluster = (parent_entry->fstClusHi<<16)|parent_entry->fstClusLo;
	directories_in_cluster_mask = fat32_desc->sectors_per_cluster*fat32_desc->bytes_per_sector/sizeof(DirectoryEntry)-1;

	for (i = 0; i < parent_entry->fileSize/sizeof(DirectoryEntry); i++){
		if (i&directories_in_cluster_mask==0){
			cur_cluster = next_cluster;
			next_cluster = get_next_cluster(fat32_desc, next_cluster);
			read_sectors(fat32_desc->first_data_sector+(cur_cluster-2)*fat32_desc->sectors_per_cluster, fat32_desc->sectors_per_cluster, fat32_desc->cluster_buf);
		}
		directory_entry = &(((DirectoryEntry*)fat32_desc->cluster_buf)[i&directories_in_cluster_mask]);
		for (j=0; j<11; j++){
			if (cname[j]!=directory_entry->name[j]){
				break;
			}
		}
		if (j == 11){
			*res_entry = *directory_entry;
			return 0;
		}
	}
	return -1;
}

int32 fat32_get_file_size(FAT32Desc *fat32_desc, char *filename){
	int32 res;
	DirectoryEntry entry;
	entry.fstClusLo = fat32_desc->root_dir_cluster_num&0xFFFF;
	entry.fstClusHi = fat32_desc->root_dir_cluster_num>>16;
	char* start = filename;
	char* end = start;
	while (*end!='\0'){
		while (*end!='\0' && *end!='/'){
			end++;
		}
		res = get_child_directory_entry(fat32_desc, &entry, start, end-start, &entry);
		if (res!=0){
			return -1;
		}
	}
	return entry.fileSize;
}

