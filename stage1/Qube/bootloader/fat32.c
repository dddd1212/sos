#include "Qube.h"
#include "hd.h"
#include "mem.h"
#include "fat32.h"
#define __attribute__(x)
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
	//int8 buf[0x200]; // buffer for one sector
	int32 num_of_sectors;
	//int32 res;
	//if((res = read_sectors(BPB_sector, 1, buf))!=0){
	//	return res;
	//}
	int8 *buf;
	buf = (int8*)0xFFFF800000000C00;
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
	if ((fat32_desc->FAT == 0) || (fat32_desc->cluster_buf == 0)){
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
	uint32 cur_cluster;
	uint32 directories_in_cluster;
	DirectoryEntry *directory_entry;
	uint32 i,j;

	name_to_cname(name, namelen, cname);
	cur_cluster = (parent_entry->fstClusHi<<16)|parent_entry->fstClusLo;
	directories_in_cluster = fat32_desc->sectors_per_cluster*fat32_desc->bytes_per_sector / sizeof(DirectoryEntry);

	while ((cur_cluster&0xFFFFFFF) < 0xFFFFFF8){
		read_sectors(fat32_desc->num_of_hidden_sectors + fat32_desc->first_data_sector + (cur_cluster - 2)*fat32_desc->sectors_per_cluster, fat32_desc->sectors_per_cluster, fat32_desc->cluster_buf);
		for (i = 0; i < directories_in_cluster; i++) {
			directory_entry = &(((DirectoryEntry*)fat32_desc->cluster_buf)[i]);
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
		cur_cluster = get_next_cluster(fat32_desc, cur_cluster);
	}
	return -1;
}

static int32 fat32_get_entry(FAT32Desc *fat32_desc, char *filename, DirectoryEntry* file_entry) {
	DirectoryEntry entry;
	entry.fstClusLo = fat32_desc->root_dir_cluster_num & 0xFFFF;
	entry.fstClusHi = fat32_desc->root_dir_cluster_num >> 16;
	char* start = filename;
	char* end = start;
	while (*end != '\0') {
		while (*end != '\0' && *end != '/') {
			end++;
		}
		if (get_child_directory_entry(fat32_desc, &entry, start, end - start, &entry) != 0) {
			return -1;
		}
		if (*end != '\0') {
			start = end + 1;
			end = start;
		}
	}
	*file_entry = entry;
	return 0;
}

int32 fat32_get_file_size(FAT32Desc *fat32_desc, char *filename){
	DirectoryEntry entry;
	if (fat32_get_entry(fat32_desc, filename, &entry) == -1) {
		return -1;
	}
	return entry.fileSize;
}

int32 fat32_read_file(FAT32Desc *fat32_desc, char *filename, int8* out_buf) {
	uint32 cur_cluster;
	uint32 byte_to_read;
	uint32 cluster_size;
	cluster_size = fat32_desc->sectors_per_cluster*fat32_desc->bytes_per_sector;
	DirectoryEntry entry;
	if (fat32_get_entry(fat32_desc, filename, &entry) == -1) {
		return -1;
	}
	cur_cluster = (entry.fstClusHi << 0x10) | entry.fstClusLo;
	byte_to_read = entry.fileSize;
	while (byte_to_read >= cluster_size) {
		read_sectors(fat32_desc->num_of_hidden_sectors + fat32_desc->first_data_sector + (cur_cluster - 2)*fat32_desc->sectors_per_cluster, fat32_desc->sectors_per_cluster, out_buf);
		out_buf += cluster_size;
		byte_to_read -= cluster_size;
		cur_cluster = get_next_cluster(fat32_desc, cur_cluster);
	}
	if (byte_to_read) {
		read_sectors(fat32_desc->num_of_hidden_sectors + fat32_desc->first_data_sector + (cur_cluster - 2)*fat32_desc->sectors_per_cluster, fat32_desc->sectors_per_cluster, fat32_desc->cluster_buf);
		for (uint32 i = 0; i < byte_to_read; i++) {
			out_buf[i] = fat32_desc->cluster_buf[i];
		}
	}
}
