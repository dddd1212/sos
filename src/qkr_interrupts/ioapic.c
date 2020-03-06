#include "ioapic.h"
#include "../MemoryManager/heap.h"
#include "../qkr_acpi/acpi.h"
#include "../screen/screen.h"
#include "../libc/string.h"
#include "../MemoryManager/memory_manager.h"
IOAPICControl g_ioapic_control;


void write_to_ioapic_register(IOAPIC * ioapic, uint32 address, uint32 data) {
	*((uint32 *)(ioapic->regs_address + IOAPICREGSEL)) = address;
	*((uint32 *)(ioapic->regs_address + IOAPICWIN)) = data;
	return;
}
uint32 read_from_ioapic_register(IOAPIC * ioapic, uint32 address) {
	*((uint32 *)(ioapic->regs_address + IOAPICREGSEL)) = address;
	return *((uint32 *)(ioapic->regs_address + IOAPICWIN));
}
QResult parse_apic_table(IOAPICControl * ioapic) {
	ACPITable * apic_table = get_acpi_table("APIC");
	uint8 * data = apic_table->entry.data;
	dump_table(apic_table);
	uint32 limit = apic_table->entry.Length - sizeof(ACPISDTHeader);
	uint32 idx = 8; // skip the first 8 bytes.
	while (idx < limit) {
		char type = data[idx];
		char len = data[idx+1];
		// TODO: ?? not really todo, but we don't check for overflows here.
		if (type == TT_IOAPIC) {
			ASSERT(len == 12);
			if (ioapic->num_of_ioapics >= MAX_IOAPICS) {
				screen_printf("ERROR! TOO MANY IOAPICS!", 0, 0, 0, 0);
				return QFail;
			}
			IOAPIC * cur = &ioapic->ioapics[ioapic->num_of_ioapics];
			cur->apic_id = data[idx + 2];

			void * addr = commit_pages(KHEAP, PAGE_SIZE);
			if (addr == NULL) return QFail;
			assign_committed(addr, PAGE_SIZE, *((uint32*)(&data[idx + 4])));
			cur->regs_address = (uint64)addr;
			cur->first_irq = *((uint32*)(&data[idx + 8]));
			
			ioapic->num_of_ioapics++;
		} else if (type == TT_INTERRUPT_SOURCE_OVERRID) {
			ASSERT(len == 10);
			ASSERT(data[idx + 2] == 0); // BUS. need to be 0 (ISA bus).
			uint8 source = data[idx + 3];
			ASSERT(source < 16);
			
			uint32 global_system_int = *((uint32*)(&data[idx + 4]));
			ioapic->isa_map[source] = global_system_int;
			uint16 flags = *((uint16*)(&data[idx + 8]));
			if (flags != 0 && flags != 5) { // 0 is the default, 5 is edge-triggerd, active-high - that is the ISA interrupt regular config.
				screen_printf("ERROR! NOT IMPLEMENTED! flags not 0 or 5!\n", 0, 0, 0, 0);
				return QFail;
			}
		} else if (type != TT_PROCESSOR_LOCAL_APIC) {
			screen_printf("WARNING! got APIC table with unknown type: %d\n", type, 0, 0, 0);
		}
		idx += len;
	}
	return QSuccess;
}

BOOL ioapic_start()
{
	IOAPICControl * ioapic = &g_ioapic_control;
	memset(ioapic, 0, sizeof(IOAPICControl));
	for (uint8 i = 0; i < 16; i++) {
		ioapic->isa_map[i] = i;
	}
	if (parse_apic_table(ioapic) == QFail) return FALSE;
	
	// Fill the rest of the fields, and disable every interrupt:
	for (uint32 i = 0; i < ioapic->num_of_ioapics; i++) {
		IOAPIC * cur = &ioapic->ioapics[i];
		ASSERT(cur->apic_id == read_from_ioapic_register(cur, IOAPIC_ID));		
		uint32 ver_and_size = read_from_ioapic_register(cur, IOAPIC_VERSION);
		cur->apic_version = ver_and_size & 0xff;
		cur->max_redirection_entries = ((ver_and_size >> 16) & 0xff) + 1;
		for (uint32 j = 0; j < cur->max_redirection_entries; j++) {
			write_to_ioapic_register(cur, IOAPIC_REDTBL_START_LOW + 2*j, read_from_ioapic_register(cur, IOAPIC_REDTBL_START_LOW) | 0x10000); // clear the mask field.
		}
	}

	//TODO: get_acpi_table("PCMP"); // multi processors acpi table.
	return TRUE;
}



IOAPIC * _get_ioapic(uint32 global_system_interrupt) {
	IOAPICControl * ioapics = &g_ioapic_control;
	
	for (uint32 i = 0; i < ioapics->num_of_ioapics; i++) {
		uint32 first_irq = ioapics->ioapics[i].first_irq;
		uint32 size = ioapics->ioapics[i].max_redirection_entries;
		if (first_irq <= global_system_interrupt && global_system_interrupt < first_irq + size) { // We found the right ioapic!
			return &ioapics->ioapics[i];
		}
	}
	return NULL;
}
BOOL _is_interrupt_enable(IOAPIC * ioapic, uint32 index_in_ioapic) {
	return ((read_from_ioapic_register(ioapic, IOAPIC_REDTBL_START_LOW + 2 * index_in_ioapic) & 0x10000) == 0);
}


QResult configure_ioapic_interrupt(uint32 global_system_interrupt, enum InterruptVectors redirect_isr, enum DeliveryMode delivery_mode, uint32 cpu_id) {
	IOAPIC * ioapic = _get_ioapic(global_system_interrupt);
	if (ioapic == NULL) return QFail;
	uint32 index = global_system_interrupt - ioapic->first_irq;
	
	if (_is_interrupt_enable(ioapic, index)) {
		screen_printf("ERROR! try to configure ioapic that already started! GSI=%d, r_isr=%d, deliver_mode = %d", global_system_interrupt, redirect_isr, delivery_mode, 0);	
		return QFail;
	}
	// configure it (do not enable it)!
	write_to_ioapic_register(ioapic, IOAPIC_REDTBL_START_HIGH + 2 * index, cpu_id << 24);
	
	write_to_ioapic_register(ioapic, IOAPIC_REDTBL_START_LOW + 2 * index, ((uint8)redirect_isr) | (((uint8)delivery_mode) << 8) | (0<<11) | (0<<12) | (0<<13) | (0<<14) | (0<<15));
	return QSuccess;
}

QResult enable_ioapic_interrupt(uint32 global_system_interrupt) {
	IOAPIC * ioapic = _get_ioapic(global_system_interrupt);
	if (ioapic == NULL) return QFail;
	uint32 index = global_system_interrupt - ioapic->first_irq;
	
	if (_is_interrupt_enable(ioapic, index)) {
		return QSuccess;
	}
	// enable it!
	write_to_ioapic_register(ioapic, IOAPIC_REDTBL_START_LOW + 2 * index, read_from_ioapic_register(ioapic, IOAPIC_REDTBL_START_LOW + 2 * index) & 0xfffeffff);
	return QSuccess;
}

QResult disable_ioapic_interrupt(uint32 global_system_interrupt) {
	IOAPIC * ioapic = _get_ioapic(global_system_interrupt);
	if (ioapic == NULL) return QFail;
	uint32 index = global_system_interrupt - ioapic->first_irq;

	if (!_is_interrupt_enable(ioapic, index)) {
		return QSuccess;
	}
	// disable it!
	write_to_ioapic_register(ioapic, IOAPIC_REDTBL_START_LOW + 2 * index, read_from_ioapic_register(ioapic, IOAPIC_REDTBL_START_LOW + 2 * index) | 0x10000);
	return QSuccess;
}

QResult configure_isa_interrupt(enum IsaInterrupts isa_interrupt, enum InterruptVectors redirect_isr, enum DeliveryMode delivery_mode, uint32 cpu_id) {
	if (g_ioapic_control.isa_map[isa_interrupt] == 0) return QFail;
	return configure_ioapic_interrupt(g_ioapic_control.isa_map[isa_interrupt], redirect_isr, delivery_mode, cpu_id);
}
QResult enable_isa_interrupt(enum IsaInterrupts isa_interrupt) {
	if (g_ioapic_control.isa_map[isa_interrupt] == 0) return QFail;
	return enable_ioapic_interrupt(g_ioapic_control.isa_map[isa_interrupt]);
}
QResult disable_isa_interrupt(enum IsaInterrupts isa_interrupt) {
	if (g_ioapic_control.isa_map[isa_interrupt] == 0) return QFail;
	return disable_ioapic_interrupt(g_ioapic_control.isa_map[isa_interrupt]);
}