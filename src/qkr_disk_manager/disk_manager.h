#ifndef DISK_MANAGER
#define DISK_MANAGER
#include "../Common/Qube.h"
#include "../qkr_qbject_manager/Qbject.h"
#include "../qkr_memory_manager/memory_manager.h"
#include "../qkr_libc/string.h"
EXPORT QResult qkr_main(KernelGlobalData* kgd);
EXPORT QResult get_file_size(QHandle qbject);
//EXPORT QResult add_disk(Qbject* disk);
#endif