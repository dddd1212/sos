#ifndef __acpi_h__
#define __acpi_h__
#include "../Common/Qube.h"

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
} ACPISDTHeader;

QResult init_acpi();


#endif // __acpi_h__