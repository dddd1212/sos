#include "device_manager.h"
#include "../screen/screen.h"
#include "../libc/stdio.h"
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

typedef struct {
	uint64 pcie_configuration_space; //physical addres
}PCIE_QNODE_CONTEXT;

uint64 g_pcie_base_address;
QHandle pcie_create_qbject(void* qnode_context, char* path, ACCESS access, uint32 flags) {
	QHandle qhandle = allocate_qbject(0);
	return qhandle;
}

QResult pcie_get_property(QHandle qbject, uint32 id, QbjectProperty* out) {
	if (id == PCIE_CONFIGURATION_SPACE) {
		*((uint64*)out) = ((PCIE_QNODE_CONTEXT*)get_qbject_associated_qnode_context(qbject))->pcie_configuration_space;
	}
	return QSuccess;
}

QResult enum_devices();
QResult qkr_main(KernelGlobalData* kgd) {
	enum_devices(); // maybe we should call this function after the boot ends.
	return QSuccess;
}


QResult pcie_bus_enum(uint32 bus_num) {
	// TODO: we assuming that everything has already configed by the bios.
	PciConfigSpace* config_space = (PciConfigSpace*)commit_pages(KHEAP, sizeof(PciConfigSpace));
	for (uint32 device_num = 0;device_num < 32; device_num++) {
		assign_committed(config_space,sizeof(PciConfigSpace), (uint64)(g_pcie_base_address + bus_num*(1<<20) + device_num*(1 << 15)));
		if (config_space->vendor_id != 0xFFFF) {
			uint32 num_of_funcs_to_enum = config_space->header_type & 0x80 ? 8 : 1; // if it is multifunction device we need to check all the 8 possible function number
			for (uint32 func_num = 0; func_num < num_of_funcs_to_enum; func_num++) {
				assign_committed(config_space, sizeof(PciConfigSpace), (uint64)(g_pcie_base_address + bus_num*(1 << 20) + device_num*(1 << 15) + func_num*(1<<12)));
				if (config_space->class_code == CLASS_CODE_BRIDGE_DEVICE && config_space->subclass == SUBCLASS_CODE_PCI_TO_PCI_BRIDGE) { // TODO: do we need to verify the PROG IF?
					pcie_bus_enum(config_space->type1_header.secondary_bus_number);
				}
				else {
					char x[100] = { "Devices/PCIe/00_00_00\0" };
					x[14] += bus_num;
					x[16] += device_num/10;
					x[17] += device_num % 10;
					x[20] += func_num;
					screen_printf("QNode created: %s (deviceID=%d)\n", x, config_space->device_id,0,0);
					//sprintf(x, "Devices/PCIe/%d_%d_%d", bus_num, device_num, func_num);
					create_qnode(x);
					PCIE_QNODE_CONTEXT* pcie_qnode_context = (PCIE_QNODE_CONTEXT*) kheap_alloc(sizeof(PCIE_QNODE_CONTEXT));
					pcie_qnode_context->pcie_configuration_space = (uint64)(g_pcie_base_address + bus_num*(1 << 20) + device_num*(1 << 15) + func_num*(1 << 12));
					QNodeAttributes attrs;
					attrs.create_qbject = pcie_create_qbject;
					attrs.read = NULL;
					attrs.write = NULL;
					attrs.qnode_context = pcie_qnode_context;
					attrs.get_property = pcie_get_property;
					set_qnode_attributes(x, &attrs);
				}
			}
		}
	}
	free_pages(config_space);
	return QSuccess;
}

/*void* pcie_bus_get_property(Qbject* qbject, uint32 id) {
	if (id == BUS_ENUM_PROPERTY) {
		return (void*)pcie_bus_enum;
	}
}*/

QResult enum_devices() {
	ACPITable* mcfg_table = get_acpi_table("MCFG");
	if (mcfg_table->entry.Length != 60) {
		screen_write_string("multiple host controller in MCFG table is not supported.", TRUE);
		return QFail;
	}
	g_pcie_base_address = *(uint64*)&mcfg_table->entry.data[8 + 0];
	//uint16 segment_group_number = *(uint16*)&mcfg_table->entry.data[8 + 8];
	//uint8 start_bus_num = *(uint8*)&mcfg_table->entry.data[8 + 10];
	//uint8 end_bus_num = *(uint8*)&mcfg_table->entry.data[8 + 11];
	create_qnode("Devices/PCIe");
	pcie_bus_enum(0);
	
}