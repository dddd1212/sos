#include "fat32.h"
static QHandle fat32_create_qbject(void* qnode_context, char* path, ACCESS access, uint32 flags);
static QResult fat32_read(QHandle qbject, uint8* buffer, uint64 position, uint64 num_of_bytes_to_read, uint64* res_num_read);
static QResult fat32_write(QHandle qbject, uint8* buffer, uint64 position, uint64 num_of_bytes_to_write, uint64* res_num_written);
typedef struct {
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

typedef struct {
	QHandle raw_disk;
	SpinLock lock;
	uint16 bytes_per_sector;
	uint8 sectors_per_cluster;
	uint16 num_of_reserved_sectors;
	uint8 num_of_FATs;
	uint32 total_num_of_sectors;
	uint32 num_of_sectors_in_FAT;
	uint32 root_dir_cluster_num;
	uint32 first_data_sector;
	uint32 *FAT;

	uint8 *cluster_buf;
}FAT32QnodeContext;

typedef struct {
	DirectoryEntry file_entry;
} FAT32QbjectContent;

QResult create_fat32(QHandle * raw_disk, QNodeAttributes * fs_qnode_attrs)
{
	FAT32QnodeContext* qnode_context = (FAT32QnodeContext*) kheap_alloc(sizeof(FAT32QnodeContext));
	if (qnode_context == NULL) {
		return QFail;
	}
	qnode_context->raw_disk = raw_disk;

	uint8 buf[0x200];
	uint64 num_read = 0;
	read_qbject(raw_disk, buf, 0, 0x200, &num_read);

	qnode_context->bytes_per_sector = *((uint16*)&buf[11]);
	qnode_context->sectors_per_cluster = *((uint8*)&buf[13]);
	qnode_context->num_of_reserved_sectors = *((uint16*)&buf[14]);
	qnode_context->num_of_FATs = *((uint8*)&buf[16]);
	qnode_context->total_num_of_sectors = *((uint32*)&buf[32]);
	qnode_context->num_of_sectors_in_FAT = *((uint32*)&buf[36]);
	qnode_context->root_dir_cluster_num = *((uint32*)&buf[44]);

	qnode_context->first_data_sector = qnode_context->num_of_reserved_sectors + qnode_context->num_of_sectors_in_FAT*qnode_context->num_of_FATs;
	uint32 num_of_sectors = qnode_context->total_num_of_sectors - qnode_context->first_data_sector;
	qnode_context->FAT = (uint32*)kheap_alloc(num_of_sectors * sizeof(uint32) / qnode_context->sectors_per_cluster);
	if (qnode_context->FAT == NULL) {
		kheap_free(qnode_context);
		return QFail;
	}
	qnode_context->cluster_buf = (uint8*)kheap_alloc(qnode_context->sectors_per_cluster*qnode_context->bytes_per_sector);
	if (qnode_context->cluster_buf == NULL) {
		kheap_free(qnode_context->FAT);
		kheap_free(qnode_context);
		return QFail;
	}
	read_qbject(raw_disk,(uint8*) qnode_context->FAT, 0x200*qnode_context->num_of_reserved_sectors, num_of_sectors*0x200, &num_read);

	fs_qnode_attrs->create_qbject = fat32_create_qbject;
	fs_qnode_attrs->get_property = NULL;
	fs_qnode_attrs->qnode_context = (void*)qnode_context;
	fs_qnode_attrs->read = fat32_read;
	fs_qnode_attrs->write = fat32_write;
	return QSuccess;
}

static inline void name_to_cname(char *name, int32 namelen, char *cname) {
	int32 i, j;
	for (i = 0; i<8 && i<namelen;i++) {
		if (name[i] == '.') {
			break;
		}
		cname[i] = name[i];
	}

	for (j = i;j<8;j++) {
		cname[j] = ' ';
	}

	if (name[i] == '.') {
		i++;
	}

	for (j = 0; j<3 && (i + j)<namelen;j++) {
		cname[8 + j] = name[i + j];
	}

	for (;j<3;j++) {
		cname[j] = ' ';
	}
}

static int32 get_next_cluster(FAT32QnodeContext* qnode_context, uint32 cluster) {
	return qnode_context->FAT[cluster]; // 2 is the first data cluster.
}

static QResult get_child_directory_entry(FAT32QnodeContext* qnode_context, DirectoryEntry *parent_entry, char *name, int32 namelen, DirectoryEntry *res_entry) {
	char cname[11];
	uint32 cur_cluster;
	uint32 directories_in_cluster;
	DirectoryEntry *directory_entry;
	uint32 i, j;

	name_to_cname(name, namelen, cname);
	cur_cluster = (parent_entry->fstClusHi << 16) | parent_entry->fstClusLo;
	directories_in_cluster = qnode_context->sectors_per_cluster*qnode_context->bytes_per_sector / sizeof(DirectoryEntry);

	while ((cur_cluster & 0xFFFFFFF) < 0xFFFFFF8) {
		uint64 num_read;
		read_qbject(qnode_context->raw_disk, qnode_context->cluster_buf, (qnode_context->first_data_sector + (cur_cluster - 2)*qnode_context->sectors_per_cluster) * 0x200, qnode_context->sectors_per_cluster * 0x200, &num_read);
		for (i = 0; i < directories_in_cluster; i++) {
			directory_entry = &(((DirectoryEntry*)qnode_context->cluster_buf)[i]);
			for (j = 0; j<11; j++) {
				if (cname[j] != directory_entry->name[j]) {
					break;
				}
			}
			if (j == 11) {
				*res_entry = *directory_entry;
				return QSuccess;
			}
		}
		cur_cluster = get_next_cluster(qnode_context, cur_cluster);
	}
	return QFail;
}

static QHandle fat32_create_qbject(void* qnode_context_a, char* filename, ACCESS access, uint32 flags) {
	FAT32QnodeContext* qnode_context = (FAT32QnodeContext*)qnode_context_a;
	spin_lock(&qnode_context->lock);
	DirectoryEntry entry;
	entry.fstClusLo = qnode_context->root_dir_cluster_num & 0xFFFF;
	entry.fstClusHi = qnode_context->root_dir_cluster_num >> 16;
	char* start = filename;
	char* end = start;
	while (*end != '\0') {
		while (*end != '\0' && *end != '/') {
			end++;
		}
		if (get_child_directory_entry(qnode_context, &entry, start, end - start, &entry) != 0) {
			spin_unlock(&qnode_context->lock);
			return NULL;
		}
		if (*end != '\0') {
			start = end + 1;
			end = start;
		}
	}
	QHandle qhandle = allocate_qbject(sizeof(FAT32QbjectContent));
	FAT32QbjectContent* qbject_content = (FAT32QbjectContent*)get_qbject_content(qhandle);
	qbject_content->file_entry = entry;
	spin_unlock(&qnode_context->lock);
	return qhandle;
}

static QResult fat32_read(QHandle qbject, uint8* buffer, uint64 position, uint64 num_of_bytes_to_read, uint64* res_num_read) {
	uint32 cur_cluster;
	uint32 byte_to_read;
	uint32 cluster_size;
	uint64 num_read;
	FAT32QnodeContext* qnode_context = (FAT32QnodeContext*)(((Qbject*)qbject)->associated_qnode->qnode_context);
	spin_lock(&qnode_context->lock);
	QHandle raw_disk = qnode_context->raw_disk;

	cluster_size = qnode_context->sectors_per_cluster*qnode_context->bytes_per_sector;
	DirectoryEntry* entry = &((FAT32QbjectContent*)get_qbject_content(qbject))->file_entry;
	cur_cluster = (entry->fstClusHi << 0x10) | entry->fstClusLo;
	byte_to_read = entry->fileSize<num_of_bytes_to_read?entry->fileSize:num_of_bytes_to_read;

	while (byte_to_read >= cluster_size) {
		read_qbject(raw_disk, buffer, (qnode_context->first_data_sector + (cur_cluster - 2)*qnode_context->sectors_per_cluster) * 0x200, qnode_context->sectors_per_cluster * 0x200, &num_read);
		buffer += cluster_size;
		byte_to_read -= cluster_size;
		cur_cluster = get_next_cluster(qnode_context, cur_cluster);
	}
	if (byte_to_read) {
		read_qbject(raw_disk, qnode_context->cluster_buf, (qnode_context->first_data_sector + (cur_cluster - 2)*qnode_context->sectors_per_cluster) * 0x200, qnode_context->sectors_per_cluster * 0x200, &num_read);
		for (uint32 i = 0; i < byte_to_read; i++) {
			buffer[i] = qnode_context->cluster_buf[i];
		}
	}
	*res_num_read = byte_to_read;
	spin_unlock(&qnode_context->lock);
	return QSuccess;
}

static QResult fat32_write(QHandle qbject, uint8* buffer, uint64 position, uint64 num_of_bytes_to_write, uint64* res_num_written) {
	return QFail;
}