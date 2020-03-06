#include "acpi.h"
#include "../qkr_screen/screen.h"
#include "../qkr_libc/string.h"
#include "../qkr_memory_manager/memory_manager.h"
#include "../qkr_memory_manager/physical_memory.h"
#include "../qkr_memory_manager/heap.h"
ACPITable * g_tables_head;
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
	ver = rsdp_desc->firstPart.Revision;
	if (ver != 0 && ver != 2) { // ver must be 0 or 2.
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
	
	if (ver == 0) {
		rsdt_phys = (uint64)rsdp_desc->firstPart.RsdtAddress; // In ACPI1, we need to use this pointer.
	} else { 
		if (checksum(((uint8*)rsdp_desc) + sizeof(RSDPDescriptor), sizeof(RSDPDescriptor20) - sizeof(RSDPDescriptor)) != QSuccess) {
			screen_printf("ERROR: RSDPDescriptor checksum is not 0!\n", 0, 0, 0, 0);
			ret = QFail;
			goto end;
		}
		rsdt_phys = rsdp_desc->XsdtAddress; // In version 2, we need to use this pointer.
	}

	screen_printf("Found rsdt at: %x!\n", (uint64)rsdt_phys, 0, 0, 0);
	
	ACPITable * head = alloc_and_copy_table(rsdt_phys, &pmem);
	if (head == NULL) return QFail;
	first_mb = NULL; // invalid now.
	ACPITable * tail = head;
	dump_table(head);
	rsdt = &head->entry;
	if ((ver == 0 && memcmp(rsdt->Signature, "RSDT", 4) != 0) || (ver == 2 && memcmp(rsdt->Signature, "XSDT", 4)) != 0) {
		screen_printf("ERROR: rsdt/xsdt signature error!\n", 0, 0, 0, 0);
		ret = QFail;
		goto end;
	}
	
	uint64 desc_addr;
	uint8 * ptr = (uint8*)(&rsdt->data[0]);
	uint32 descs_ptr_size;
	ACPITable * entry;
	if (ver == 0) {
		descs_ptr_size = 4;
	} else {
		descs_ptr_size = 8;
	}
	screen_printf("rsdt length: %d\n", rsdt->Length, 0, 0, 0);
	for (uint32 i = 0; i < rsdt->Length - sizeof(ACPISDTHeader); i += descs_ptr_size) {
		if (ver == 0) {
			desc_addr = (uint64)(*((uint32*)(ptr + i)));
		} else {
			desc_addr = (uint64)(*((uint64*)(ptr + i)));
		}
		entry = alloc_and_copy_table(desc_addr, &pmem);
		if (entry == NULL) return QFail;
		tail->next = entry;
		entry->next = NULL;
		tail = entry;
		dump_table(entry);
	}
	g_tables_head = head;
end:
	physical_memory_fini(&pmem);
	return ret;
}

ACPITable * get_acpi_table(char * name) {
	if (strlen(name) != 4) return NULL;
	for (ACPITable * cur = g_tables_head; cur != NULL; cur = cur->next) {
		if (memcmp(name, cur->entry.Signature, 4) == 0) return cur;
	}
	return NULL;
}

ACPITable * alloc_and_copy_table(uint64 phys_mem_table, PhysicalMemory * pmem) {
	if (pmem == NULL) { // if the user didn't give an object, the function will be slower.
		PhysicalMemory pmem_s;
		physical_memory_init(&pmem_s);
		ACPITable * ret = alloc_and_copy_table(phys_mem_table, &pmem_s);
		physical_memory_fini(&pmem_s);
		return ret;
	}
	ACPISDTHeader * entry = physical_memory_get_ptr(pmem, phys_mem_table, sizeof(ACPISDTHeader));
	if (entry == NULL) return NULL;
	entry = physical_memory_get_ptr(pmem, phys_mem_table, entry->Length);
	if (entry == NULL) return NULL;
	if (checksum((uint8*)entry, entry->Length) != QSuccess) {
		screen_printf("ERROR: table checksum is not 0!\n", 0, 0, 0, 0);
		return NULL;
	}
	ACPITable * ret = kheap_alloc(sizeof(ACPITable) + entry->Length - sizeof(ACPISDTHeader));
	if (ret == NULL) return NULL;
	ret->next = NULL;
	memcpy(&ret->entry, entry, entry->Length);
	return ret;
}

void dump_table(ACPITable * table) {
	/*
	table->entry.CreatorID;
	table->entry.CreatorRevision;
	table->entry.Length;
	table->entry.OEMID;
	table->entry.OEMRevision;
	table->entry.OEMTableID;
	table->entry.Revision;
	table->entry.Signature;
	*/
	char temp[10], temp2[10];
	memcpy(temp, table->entry.Signature, sizeof(table->entry.Signature));
	temp[sizeof(table->entry.Signature)] = '\0';
	screen_printf("ACPITable: name=%s. Revision: %d, Length: %d.\n", (uint64)&temp[0], table->entry.Revision, table->entry.Length, 0);
	memcpy(temp, table->entry.OEMID, sizeof(table->entry.OEMID));
	temp[sizeof(table->entry.OEMID)] = '\0';
	memcpy(temp2, table->entry.OEMTableID, sizeof(table->entry.OEMTableID));
	temp2[sizeof(table->entry.OEMTableID)] = '\0';
	screen_printf("    (OEM: id:%s, table_id:%s, revision:%d)\n", (uint64)&temp[0], (uint64)&temp2[0], table->entry.OEMRevision, 0);
	memcpy(temp, &table->entry.CreatorID, sizeof(table->entry.CreatorID));
	temp[sizeof(table->entry.CreatorID)] = '\0';
	screen_printf("    (Creator: id:%s, revision:%d).\n", (uint64)&temp[0], table->entry.CreatorRevision, 0, 0);
	for (uint32 i = 0; i < table->entry.Length - sizeof(ACPISDTHeader); i++) {
		screen_printf("%02x ", (uint64)(table->entry.data[i]), 0, 0, 0);
		if (i + 1 <= table->entry.Length - sizeof(ACPISDTHeader)) {
			if (i % 16 == 7) screen_write_string("   ", FALSE);
			if (i % 16 == 15) screen_new_line();
		}
	}
	screen_new_line();
	return;
}
