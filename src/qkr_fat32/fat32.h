#ifndef FAT32_H
#define FAT32_H
#include "../Common/Qube.h"
#include "../qkr_qbject_manager/Qbject.h"
#include "../qkr_memory_manager/memory_manager.h"
#include "../Common/spin_lock.h"
EXPORT QResult create_fat32(QHandle* raw_disk, QNodeAttributes* fs_qnode_attrs);
#endif