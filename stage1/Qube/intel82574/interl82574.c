#include "intel82574.h"
typedef struct {
	uint16 vendor_id;
	uint16 device_id;
	uint16 command;
	uint16 status;
	uint8 revision_id;
	uint8 prof_if;
	uint8 subclass;
	uint8 class_code;
	uint8 cache_line_size;
	uint8 latency_timer;
	uint8 header_type;
	uint8 bist;
	union {
		struct {
			uint32 bar0;
			uint32 bar1;
			uint32 bar2;
			uint32 bar3;
			uint32 bar4;
			uint32 bar5;
			uint32 cardbus_cis_pointer;
			uint16 subsystem_vendor_id;
			uint16 subsystem_id;
			uint32 exp_rom_base_address;
			uint8 capabilities_pointer;
			uint8 reserved[7];
			uint8 interrupt_line;
			uint8 interrupt_pin;
			uint8 min_grant;
			uint8 max_latency;
		} __attribute__((packed)) type0_header;
		struct {
			uint32 bar0;
			uint32 bar1;
			uint8 primary_bus_number;
			uint8 secondary_bus_number;
			uint8 subordinate_bus_number;
			uint8 secondary_latency_timer;
			uint8 io_base;
			uint8 io_limit;
			uint16 secondary_status;
			uint16 memory_base;
			uint16 memory_limit;
			// TODO: complete this struct!
		} __attribute__((packed)) type1_header;
	};

} __attribute__((packed)) PciConfigSpace;
QResult qkr_main(KernelGlobalData * kgd) {
	QHandle pci = create_qbject("Devices/PCIe/03_00_00\0", 0);
	uint64 configuration_space_phys_addr;
	get_qbject_property(pci, PCIE_CONFIGURATION_SPACE, (QbjectProperty*)&configuration_space_phys_addr);
	PciConfigSpace* config_space = (PciConfigSpace*)commit_pages(KHEAP, sizeof(PciConfigSpace));
	assign_committed(config_space, sizeof(PciConfigSpace), configuration_space_phys_addr);
	screen_printf("BAR0=0x%x\nBAR1=0x%x\nBAR2=0x%x\nBAR3=0x%x\n", config_space->type0_header.bar0, config_space->type0_header.bar1, config_space->type0_header.bar2, config_space->type0_header.bar3);
	return QSuccess;
}
