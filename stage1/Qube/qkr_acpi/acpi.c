#include "acpi.h"
#include "../screen/screen.h"
#include "../libc/string.h"
#include "../MemoryManager/memory_manager.h"

QResult qkr_main(KernelGlobalData * kgd) {
	init_acpi();
	return QSuccess;
}


QResult init_acpi() {
	char * first_mb = (char*) commit_pages(MODULES, 1000 * 1000);
	if (first_mb == NULL) return QFail;
	assign_committed(first_mb, 0x100000, 0);

	RSDPDescriptor * rsdp_desc = NULL;
	char * sign = "RSD PTR ";
	for (int i = 0; i < 1024; i += 16) {
		if (memcmp(first_mb + i, sign, 8) == 0) {
			screen_printf("Found1 at %x\n", i, 0, 0, 0);
			if (rsdp_desc) {
				screen_write_string("MORE1 THEN ONE RSDP!! ERROR!", TRUE);

				return QFail;
			}
			rsdp_desc = (RSDPDescriptor *)(first_mb + i);
		}
	}
	if (rsdp_desc == NULL) {
		for (int i = 0xe0000; i < 0x100000; i += 16) {
			if (memcmp(first_mb + i, sign, 8) == 0) {
				screen_printf("Found2 at %x\n", i, 0, 0, 0);
				if (rsdp_desc) {
					screen_write_string("MORE2 THEN ONE RSDP!! ERROR!", TRUE);
					return QFail;
				}
				rsdp_desc = (RSDPDescriptor *)(first_mb + i);
			}
		}
	}
	if (rsdp_desc == NULL) {
		screen_write_string("ERROR! Cant find the rsdp!",TRUE);
		return QFail;
	}
	while (1) {}
}
