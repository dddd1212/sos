#include "acpi.h"
#include "../screen/screen.h"
#include "../libc/string.h"
#include "../MemoryManager/memory_manager.h"
#include "../MemoryManager/physical_memory.h"
QResult qkr_main(KernelGlobalData * kgd) {
	init_acpi();
	return QSuccess;
}

#define MEGABYTE 0x100000

QResult checksum(uint8 * addr, uint64 size) {
	uint8 sum = 0;
	uint8 * end = addr + size;
	for (uint8 * i = addr; i < end; i++) {
		sum += *i;
	}
	if (sum == 0) return QSuccess;
	return QFail;

}
QResult init_acpi() {
	PhysicalMemory pmem;
	PhysicalMemory pmem2;
	if (physical_memory_init(&pmem) == QFail) return QFail;
	if (physical_memory_init(&pmem2) == QFail) return QFail;
	QResult ret = QSuccess;
	uint32 ver;
	char * first_mb = physical_memory_get_ptr(&pmem, 0, MEGABYTE);
	if (first_mb == NULL) return QFail;
	
	RSDPDescriptor20 * rsdp_desc = NULL;
	char * sign = "RSD PTR ";
	for (int i = 0; i < 1024; i += 16) {
		if (memcmp(first_mb + i, sign, 8) == 0) {
			screen_printf("Found1 at %x\n", i, 0, 0, 0);
			if (rsdp_desc) {
				screen_write_string("MORE1 THEN ONE RSDP!! ERROR!", TRUE);
				ret = QFail;
				goto end;
			}
			rsdp_desc = (RSDPDescriptor20 *)(first_mb + i);
		}
	}
	if (rsdp_desc == NULL) {
		for (int i = 0xe0000; i < MEGABYTE; i += 16) {
			if (memcmp(first_mb + i, sign, 8) == 0) {
				screen_printf("Found2 at %x\n", i, 0, 0, 0);
				if (rsdp_desc) {
					screen_write_string("MORE2 THEN ONE RSDP!! ERROR!", TRUE);
					ret = QFail;
					goto end;
				}
				rsdp_desc = (RSDPDescriptor20 *)(first_mb + i);
			}
		}
	}
	if (rsdp_desc == NULL) {
		screen_write_string("ERROR! Cant find the rsdp!",TRUE);
		ret = QFail;
		goto end;
	}

	// Validate acpi version 1 or 2:
	ver = rsdp_desc->firstPart.Revision + 1;
	if (!(ver - 1 <= 1)) { // ver must be 1 or 2.
		screen_printf("ERROR: ACPI Revision not match: %d\n", (uint64)rsdp_desc->firstPart.Revision, 0, 0, 0);
		ret = QFail;
		goto end;
	}

	uint64 rsdt_phys;
	ACPISDTHeader* rsdt;

	// validate checksum:
	if (checksum((uint8*)rsdp_desc, sizeof(RSDPDescriptor)) != QSuccess) {
		screen_printf("ERROR: RSDPDescriptor checksum is not 0!\n", 0, 0, 0, 0);
		ret = QFail;
		goto end;
	}
	
	if (ver == 1) {
		rsdt_phys = (uint64)rsdp_desc->firstPart.RsdtAddress; // In version 1, we need to use this pointer.
	} else { 
		if (checksum(((uint8*)rsdp_desc) + sizeof(RSDPDescriptor), sizeof(RSDPDescriptor20) - sizeof(RSDPDescriptor)) != QSuccess) {
			screen_printf("ERROR: RSDPDescriptor checksum is not 0!\n", 0, 0, 0, 0);
			ret = QFail;
			goto end;
		}
		rsdt_phys = rsdp_desc->XsdtAddress; // In version 2, we need to use this pointer.
	}

	screen_printf("Found rsdt at: %x!\n", (uint64)rsdt_phys, 0, 0, 0);

	first_mb = NULL; // invalid now.
	rsdt = physical_memory_get_ptr(&pmem, rsdt_phys, sizeof(ACPISDTHeader));
	rsdt = physical_memory_get_ptr(&pmem, rsdt_phys, rsdt->Length);
	screen_printf("rsdt len: %x!\n", rsdt->Length - sizeof(ACPISDTHeader), 0, 0, 0);
	if (checksum((uint8*)rsdt, rsdt->Length) != QSuccess) {
		screen_printf("ERROR: rsdt checksum is not 0!\n", 0, 0, 0, 0);
		ret = QFail;
		goto end;
	}

	if (memcmp(rsdt->Signature, "RSDT", 4) != 0) {
		screen_printf("ERROR: rsdt signature error!\n", 0, 0, 0, 0);
		ret = QFail;
		goto end;
	}
	
	
	uint64 desc_addr;
	uint8 * ptr = (uint8*)(rsdt + 1);
	uint32 descs_ptr_size;
	if (ver == 1) {
		descs_ptr_size = 4;
	} else {
		descs_ptr_size = 8;
	}
	screen_printf("rsdt length: %d\n", rsdt->Length, 0, 0, 0);
	for (uint32 i = 0; i < rsdt->Length - sizeof(ACPISDTHeader); i += descs_ptr_size) {
		if (ver == 1) {
			desc_addr = (uint64)(*((uint32*)(ptr + i)));
		} else {
			desc_addr = (uint64)(*((uint64*)(ptr + i)));
		}
		//screen_printf("TABLE Found at: %x!\n", desc_addr, 0, 0, 0);
		ACPISDTHeader * table = physical_memory_get_ptr(&pmem2, desc_addr, sizeof(ACPISDTHeader));
		table = physical_memory_get_ptr(&pmem2, desc_addr, sizeof(ACPISDTHeader));
		table = physical_memory_get_ptr(&pmem2, desc_addr, sizeof(ACPISDTHeader));
		//screen_printf("TABLE Found at: $x. name: $x. len: $d\n", 0, 0, 0, 0);
		screen_printf("TABLE Found at: %x. name: %x. len: %d\n", desc_addr, *((int*)table->Signature), table->Length, 0);
		
	}

	
	while (1) {}
end:
	physical_memory_fini(&pmem);
	while (1) {}
	return ret;
}
