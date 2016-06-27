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

QResult init_acpi();


#endif // __acpi_h__