#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H
#include "../Common/Qube.h"
#include "../QbjectManager/Qbject.h"
#include "../MemoryManager/memory_manager.h"
#include "../qkr_acpi/acpi.h"
#include "../screen/screen.h"
#include "../Common/spin_lock.h"
typedef QResult(*BusEnumFunc)(QHandle h);
EXPORT QResult qkr_main(KernelGlobalData* kgd);
EXPORT QResult enum_bus(QHandle bus_qbject);
#endif
