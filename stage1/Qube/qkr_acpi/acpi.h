#ifndef __acpi_h__
#define __acpi_h__
#include "../Common/Qube.h"
#include "../MemoryManager/physical_memory.h"
// Note that GCC support to use these pragmas. We use it and not __attribute__((pack)) because of visual studio.
#pragma pack(push)
#pragma pack(1)



typedef struct {
	char Signature[8];
	uint8 Checksum;
	char OEMID[6];
	uint8 Revision;
	uint32 RsdtAddress;
} __attribute__((packed)) RSDPDescriptor;

typedef struct {
	RSDPDescriptor firstPart;

	uint32 Length;
	uint64 XsdtAddress;
	uint8 ExtendedChecksum;
	uint8 reserved[3];
} __attribute__((packed)) RSDPDescriptor20;


typedef struct {
	char Signature[4];
	uint32 Length;
	uint8 Revision;
	uint8 Checksum;
	char OEMID[6];
	char OEMTableID[8];
	uint32 OEMRevision;
	uint32 CreatorID;
	uint32 CreatorRevision;
	uint8 data[0]; // not part of the header.
} ACPISDTHeader;

struct _ACPITable;
typedef struct _ACPITable {
	struct _ACPITable *next;
	ACPISDTHeader entry;
} ACPITable;


QResult init_acpi();
ACPITable * alloc_and_copy_table(uint64 phys_mem_table, PhysicalMemory * inited_phys_mem);

// return a table.
EXPORT ACPITable * get_acpi_table(char * name);

// print table on the screen.
EXPORT void dump_table(ACPITable * table);

extern ACPITable * g_tables_head;

#endif // __acpi_h__