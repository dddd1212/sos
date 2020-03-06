#ifndef DISK_MANAGER
#define DISK_MANAGER
#include "../Common/Qube.h"
#include "../QbjectManager/Qbject.h"
#include "../MemoryManager/memory_manager.h"
#include "../libc/string.h"
EXPORT QResult qkr_main(KernelGlobalData* kgd);
EXPORT QResult get_file_size(QHandle qbject);
//EXPORT QResult add_disk(Qbject* disk);
#endif