#include "memory_manager.h"
uint64 *g_physical_pages_current;
uint64 *g_physical_pages_end;
uint64 *g_physical_pages_start;
MemoryRegion* g_regions[NUM_OF_REGION_TYPE];
void* init_memory_manager(KernelGlobalData* kgd) {
	g_physical_pages_current = kgd->boot_info->physical_pages_current;
	g_physical_pages_end = kgd->boot_info->physical_pages_end;
	g_physical_pages_start = kgd->boot_info->physical_pages_start;


}