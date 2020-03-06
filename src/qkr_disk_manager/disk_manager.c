#include "disk_manager.h"
#include "../qkr_fat32/fat32.h"
QHandle def_hd_create_qbject(void* qnode_context, char* path, ACCESS access, uint32 flags);
QResult def_hd_read(QHandle qbject, uint8* buffer, uint64 position, uint64 num_of_bytes_to_read, uint64* res_num_read);
QResult def_hd_write(QHandle qbject, uint8* buffer, uint64 position, uint64 num_of_bytes_to_write, uint64* res_num_written);

static QResult add_file_system(QHandle* raw_disk);

typedef struct {
	uint32 first_sector_lba;
} DefHdQNodeContext;

static void def_hd_read_raw_sectors(uint32 LBA, uint8 numOfSectors, void *out_buf) {
	uint8 read_status;
	uint32 i;
	__out8(0x1f6, (LBA >> 24) | 0b11100000);
	__out8(0x1f2, numOfSectors);
	__out8(0x1f3, (int8)LBA);
	__out8(0x1f4, (int8)(LBA >> 8));
	__out8(0x1f5, (int8)(LBA >> 16));

	__out8(0x1f7, 0x20); //command port. read with retry
	for (i = 0; i < numOfSectors; i++) {
		do {
			read_status = __in8(0x1f7);
		} while (!(read_status & 8));
		__insw(0x1f0, 0x100, (void*)((char*)out_buf + 0x200 * i));
	}
}

QResult qkr_main(KernelGlobalData * kgd) {
	QResult res;
	QNodeAttributes qnode_attrs;
	QHandle def_hd;
	uint8 buf[0x200];
	res = create_qnode("Devices/Default/HD");
	DefHdQNodeContext* qnode_context;



	qnode_context = (DefHdQNodeContext*)kheap_alloc(sizeof(DefHdQNodeContext));
	def_hd_read_raw_sectors(0, 1, buf);
	qnode_context->first_sector_lba = 0; // first try to mount as raw (without MBR header)
	qnode_attrs.qnode_context = qnode_context;
	qnode_attrs.create_qbject = def_hd_create_qbject;
	qnode_attrs.read = def_hd_read;
	qnode_attrs.write = def_hd_write;
	set_qnode_attributes("Devices/Default/HD", &qnode_attrs);
	def_hd = create_qbject("Devices/Default/HD", ACCESS_READ | ACCESS_WRITE);
	res = add_file_system(def_hd);
	if (res == QFail) {
		// if we failed, maybe there is an MBR header
		qnode_context->first_sector_lba = *((uint32*)(&buf[446 + 8]));
		res = add_file_system(def_hd);
	}
	if (res != QSuccess) {
		return QFail;
	}
	return QSuccess;
}

static QResult add_file_system(QHandle* raw_disk) {
	QResult res;
	QNodeAttributes fs_qnode_attrs;
	uint64 num_read = 0;
	uint8 buffer[0x200];
	read_qbject(raw_disk, buffer, 0, 0x200, &num_read);
	// currently we support only fat32.
	if (*((uint64*)&buffer[82]) != 0x2020203233544146) { //"FAT32   ". according to the specification, this is wrong, but for now I leave it this way.
		return QFail;
	}
	res = create_fat32(raw_disk, &fs_qnode_attrs);
	if (res != QSuccess) {
		return res;
	}

	// TODO: this is not support more than one file system and must be updated to enumerate over FS0,FS1,...
	res = create_qnode("FS/FS0");
	if (res != QSuccess) {
		return res;
	}

	set_qnode_attributes("FS/FS0", &fs_qnode_attrs);
	return QSuccess;
}

QHandle def_hd_create_qbject(void* qnode_context, char* path, ACCESS access, uint32 flags) {
	if (path[0] == '\0') {
		return allocate_qbject(0);
	}
	return NULL;
}

QResult def_hd_read(QHandle qbject, uint8* out_buf, uint64 position, uint64 num_of_bytes_to_read, uint64* res_num_read){
	uint32 num_of_sectors;
	uint32 first_sector;
	uint8* temp_buf;
	first_sector = position / 0x200;
	num_of_sectors = (position + num_of_bytes_to_read + (0x200 - 1)) / 0x200 - first_sector;
	first_sector += ((DefHdQNodeContext*)get_qbject_associated_qnode_context(qbject))->first_sector_lba;
	if (((position&(0x200 - 1)) == 0) && ((num_of_bytes_to_read&(0x200 - 1)) == 0)) {
		def_hd_read_raw_sectors(first_sector, num_of_sectors, out_buf);
		*res_num_read = num_of_bytes_to_read;
		return QSuccess;
	}
	else {
		temp_buf = kheap_alloc(0x200 * num_of_sectors);
		if (temp_buf == NULL) {
			return QFail;
		}
		def_hd_read_raw_sectors(first_sector, num_of_sectors, temp_buf);
		memcpy(temp_buf+(position&(0x200-1)), out_buf, num_of_bytes_to_read);
		kheap_free(temp_buf);
		*res_num_read = num_of_bytes_to_read;
		return QSuccess;
	}
}
QResult def_hd_write(QHandle qbject, uint8* buffer, uint64 position, uint64 num_of_bytes_to_write, uint64* res_num_written) {
	return QFail;
}

QResult get_file_size(QHandle qbject) {
	uint64 res;
	if (QSuccess != get_qbject_property(qbject, FILE_SIZE_PROPERTY, (QbjectProperty*)&res)) {
		return -1;
	}
	return res;
}