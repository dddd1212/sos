#ifndef FAT32_H
#define FAT32_H
#include "../Common/Qube.h"
#include "../QbjectManager/Qbject.h"
#include "../MemoryManager/memory_manager.h"
#include "../Common/spin_lock.h"
QResult create_fat32(QHandle* raw_disk, QNodeAttributes* fs_qnode_attrs);
#endif