#ifndef __IO_APIC_H__
#define __IO_APIC_H__
#include "../Common/Qube.h"
#include "interrupts.h"
typedef struct {
	uint32 apic_id;
	uint32 apic_version;
	uint32 max_redirection_entries;
	uint32 first_irq;
	uint64 regs_address;
} IOAPIC;

enum IsaInterrupts {
	ISA_SYSTEM_TIMER = 0,
	ISA_KEYBOARD_CONTROLLER = 1,
	ISA_CASCADED_SIGNALS = 2,
	ISA_SERIAL_PORT_CONTROLLER_2 = 3,
	ISA_SERIAL_PORT_CONTROLLER_1 = 4,
	ISA_PARALLEL_PORT_2_3 = 5,
	ISA_FLOPPY_DISK_CONTROLLER = 6,
	ISA_PARALLEL_PORT_1 = 7,
	ISA_REAL_TIME_CLOCK = 8,
	ISA_ACPI = 9,
	ISA_10 = 10,
	ISA_11 = 11,
	ISA_MOUSE = 12,
	ISA_CPU_CO_PROCESSOR = 13,
	ISA_PRIMARY_ATA = 14,
	ISA_SECONDARY_ATA = 15,
};

#define MAX_IOAPICS 2
typedef struct {
	uint32 isa_map[16];
	

	
	uint32 num_of_ioapics;
	IOAPIC ioapics[MAX_IOAPICS];
} IOAPICControl;

typedef enum {
	TT_PROCESSOR_LOCAL_APIC = 0,
	TT_IOAPIC = 1,
	TT_INTERRUPT_SOURCE_OVERRID = 2,
	TT_NMI = 3,
	TT_LAPIC_NMI = 4,
	TT_LAPIC_ADDRESS_OVERRIDE = 5,
	TT_IOSAPIC = 6,
	TT_LSAPIC = 7,
	TT_PLATFORM_INTERRUPT = 8,
} ApicTablesTypes;

#define IOAPICREGSEL 0
#define IOAPICWIN 0x10
//IO APIC Registers
typedef enum {
	IOAPIC_ID = 0,
	IOAPIC_VERSION = 1,
	IOAPIC_ARB = 2,
	IOAPIC_REDTBL_START_LOW = 0x10,
	IOAPIC_REDTBL_START_HIGH = 0x11,
} IOApicRegisters;

enum DeliveryMode {
	DM_FIXED = 0,
	DM_LOWEST_PRIORITY = 1,
	DM_SMI = 2,
	DM_NMI = 4,
	DM_INIT = 5,
	DM_EXT_INIT = 7,
};
QResult ioapic_start();

QResult parse_apic_table(IOAPICControl * ioapic);

IOAPIC * _get_ioapic(uint32 global_system_interrupt);
BOOL _is_interrupt_enable(IOAPIC * ioapic, uint32 index_in_ioapic);

void write_to_ioapic_register(IOAPIC * ioapic, uint32 address, uint32 data);
uint32 read_from_ioapic_register(IOAPIC * ioapic, uint32 address);

EXPORT QResult configure_ioapic_interrupt(uint32 global_system_interrupt, enum InterruptVectors redirect_isr, enum DeliveryMode delivery_mode, uint32 cpu_id);
EXPORT QResult enable_ioapic_interrupt(uint32 global_system_interrupt);
EXPORT QResult disable_ioapic_interrupt(uint32 global_system_interrupt);

EXPORT QResult configure_isa_interrupt(enum IsaInterrupts isa_interrupt, enum InterruptVectors redirect_isr, enum DeliveryMode delivery_mode, uint32 cpu_id);
EXPORT QResult enable_isa_interrupt(enum IsaInterrupts isa_interrupt);
EXPORT QResult disable_isa_interrupt(enum IsaInterrupts isa_interrupt);

#endif // __IO_APIC_H__
