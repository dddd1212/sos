#include "device_manager.h"
typedef struct {
	void* base_address;
	uint8 start_bus_num;
	uint8 end_bus_num;
} PcieBus;
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
} __attribute__((packed)) PciConfigSpace;
void enum_devices();
QResult qkr_main(KernelGlobalData* kgd) {
	enum_devices(); // maybe we should call this function after the boot ends.
}

QHandle pcie_bus_create_qbject(void* qnode_context, char* path, ACCESS access, uint32 flags) {
	if (path[0] == '\0') {
		return allocate_qbject(0);
	}
 	return NULL;
}

QResult pcie_bus_enum(QHandle h) {
	// TODO: we assuming that everything has already configed by the bios.
	PcieBus* pcie_bus = (PcieBus*) get_qbject_associated_qnode_context(h);
	for (uint32 device_num = 0;device_num < 32; device_num++) {
		PciConfigSpace* config_space = (PciConfigSpace*)((uint8*)pcie_bus->base_address + device_num*(1 << 15));
		if (config_space->vendor_id != 0xFFFF) {
			if (config_space->vendor_id == 0x15ad && config_space->device_id == 0x07a0) {
				PcieBus* pcie_root = kheap_alloc(sizeof(PcieBus));
				pcie_root->base_address = base_address;
				pcie_root->start_bus_num = start_bus_num;
				pcie_root->end_bus_num = end_bus_num;
				attrs.create_qbject = pcie_bus_create_qbject;
				attrs.qnode_context = pcie_root;
				attrs.get_property = pcie_bus_get_property;
			}
		}
	}
	return QSuccess;
}

void* pcie_bus_get_property(Qbject* qbject, uint32 id) {
	if (id == BUS_ENUM_PROPERTY) {
		return (void*)pcie_bus_enum;
	}
}

void enum_devices() {
	ACPITable* mcfg_table = get_acpi_table("MCFG");
	if (mcfg_table->entry.Length != 60) {
		screen_write_string("multiple host controller in MCFG table is not supported.", TRUE);
		return QFail;
	}
	uint64 base_address = *(uint64*)&mcfg_table->entry.data[8 + 0];
	//uint16 segment_group_number = *(uint16*)&mcfg_table->entry.data[8 + 8];
	uint8 start_bus_num = *(uint8*)&mcfg_table->entry.data[8 + 10];
	uint8 end_bus_num = *(uint8*)&mcfg_table->entry.data[8 + 11];
	create_qnode("Devices/RootPCIE");
	QNodeAttributes attrs;
	PcieBus* pcie_root = kheap_alloc(sizeof(PcieBus));
	pcie_root->base_address = base_address;
	pcie_root->start_bus_num = start_bus_num;
	pcie_root->end_bus_num = end_bus_num;
	attrs.create_qbject = pcie_bus_create_qbject;
	attrs.qnode_context = pcie_root;
	attrs.get_property = pcie_bus_get_property;
	set_qnode_attributes("Devices/RootPCIE", &attrs);
	
	QHandle pcie_root = create_qbject("Devices/RootPCIE", ACCESS_QNODE_MANAGEMENT);
	enum_bus(pcie_root);
}

QResult enum_bus(QHandle bus_qbject) {
	BusEnumFunc enum_func;
	QResult res = get_qbject_property(bus_qbject, BUS_ENUM_PROPERTY, &enum_func);
	if (QSuccess != res) {
		return res;
	}
	enum_func(bus_qbject);
}