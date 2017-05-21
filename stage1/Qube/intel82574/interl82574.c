#include "intel82574.h"
#define PTE(x) ((uint64*)(0xFFFFF68000000000 + (((((uint64)x) & 0x0000FFFFFFFFFFFF)>>12)<<3)))
#define RECV_DESCRIPTOR_SIZE 0x10
#define NUM_OF_RECV_DESCRIPTORS 0x100
#define DESCRIPTOR_BUFFER_SIZE 0x800
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
	QHandle pci;
	NetworkQueueManagerContext* nm_queue_context;
	PciConfigSpace* config_space;
	uint8* memoryBAR;
	uint32 current_descriptor_offset;
} NicContext;


NicContext* g_isr_context[0x100];
uint8* Gtemp;
void isr_func(ProcessorContext* x) {
	NicContext* nic_context = g_isr_context[x->interrupt_vector];
	screen_printf("network interrupt happend\n",0,0,0,0);
	*((uint32*)(nic_context->memoryBAR + 0x000c0)) = 0x84;
	notify_queue_is_not_empty(nic_context->nm_queue_context);
}


QResult get_network_data(NetworkQueueDriverContext queue_driver_context, NetworkData** out) {

}
BOOL is_queue_empty(NetworkQueueDriverContext queue_driver_context) {
	NicContext* nic_context = (NicContext*)queue_driver_context;
	return (nic_context->current_descriptor_offset == *(uint32*)(nic_context->memoryBAR + 0x2810));
}

QResult initialize_nic(NetworkInterfaceManagerContext network_interface_manager_context, NetworkInterfaceDriverContext network_interface_driver_context) {
	NicContext* nic_context = (NicContext*)network_interface_driver_context;
	QHandle pci = create_qbject("Devices/PCIe/03_00_00\0", 0);
	uint64 configuration_space_phys_addr;
	get_qbject_property(pci, PCIE_CONFIGURATION_SPACE, (QbjectProperty*)&configuration_space_phys_addr);
	PciConfigSpace* config_space = (PciConfigSpace*)commit_pages(KHEAP, sizeof(PciConfigSpace));
	assign_committed(config_space, sizeof(PciConfigSpace), configuration_space_phys_addr);
	uint8* memoryBAR = (uint8*)commit_pages(KHEAP, 0x8000);
	Gtemp = memoryBAR;
	assign_committed(memoryBAR, 0x8000, config_space->type0_header.bar0);
	
	nic_context->pci = pci;
	nic_context->config_space = config_space;
	nic_context->memoryBAR = memoryBAR;
	nic_context->current_descriptor_offset = 0;

	uint8 interrupt_vector = NETWORK_INTERRUPT; // TODO: we should get this number from some sort of PNP.
	*((uint32*)((uint8*)config_space + 0xd4)) = 0xFEE00000;
	*((uint16*)((uint8*)config_space + 0xdc)) = interrupt_vector; 
	*((uint32*)((uint8*)config_space + 0xd2)) = 0x1;

	uint8* recv_descs = (uint8*)alloc_pages(KHEAP, RECV_DESCRIPTOR_SIZE*NUM_OF_RECV_DESCRIPTORS);
	*((uint32*)(memoryBAR + 0x02800)) = (uint32)(*PTE(recv_descs))&(~3);
	*((uint32*)(memoryBAR + 0x02804)) = (uint32)(((*PTE(recv_descs))&(~3)) >> 32);
	*((uint32*)(memoryBAR + 0x02808)) = RECV_DESCRIPTOR_SIZE*NUM_OF_RECV_DESCRIPTORS;
	*((uint32*)(memoryBAR + 0x02810)) = 0;
	*((uint32*)(memoryBAR + 0x02818)) = RECV_DESCRIPTOR_SIZE*(NUM_OF_RECV_DESCRIPTORS-1);

	uint8* data = alloc_pages(KHEAP, NUM_OF_RECV_DESCRIPTORS*DESCRIPTOR_BUFFER_SIZE);
	for (uint32 i = 0; i < NUM_OF_RECV_DESCRIPTORS; i++) {
		*((uint64*)(recv_descs+RECV_DESCRIPTOR_SIZE*i)) = (*PTE(data+DESCRIPTOR_BUFFER_SIZE*i))&(~0xFFF) + (DESCRIPTOR_BUFFER_SIZE*i)&0xFFF;
	}

	register_isr(interrupt_vector, (ISR)isr_func);
	g_isr_context[interrupt_vector] = nic_context;

	*((uint32*)(memoryBAR + 0x000c8)) = 0; // ICS - Interrupt Cause Set Register
	*((uint32*)(memoryBAR + 0x000d0)) = 0x00080; // enable interrupt RXDMT 
	*((uint32*)(memoryBAR + 0x02c00)) = 0; // RSRPD - Receive Small Packet Detect Interrupt

	NetworkQueueRegisterData queue_register_data;
	queue_register_data.get_network_data = get_network_data;
	queue_register_data.is_queue_empty = is_queue_empty;
	queue_register_data.queue_driver_context = network_interface_driver_context;
	if (QSuccess != register_queue(network_interface_manager_context, &queue_register_data, &nic_context->nm_queue_context)) {
		return QFail;
	}

	*((uint32*)(memoryBAR + 0x00100)) = 0x202; // enable receive packets.
}

QResult register_nic() {
	NetworkInterfaceRegisterData interface_data;
	NicContext* nic_context = (NicContext*)kheap_alloc(sizeof(NicContext));
	interface_data.initialize = initialize_nic;
	interface_data.network_interface_driver_context = nic_context;
	register_interface(&interface_data);
}

QResult qkr_main(KernelGlobalData * kgd) {
	for (uint32 i = 0; i < sizeof(g_isr_context)/sizeof(g_isr_context[0]); i++){
		g_isr_context[i] = NULL;
	}
	register_nic();
	return QSuccess;
	/*
	QHandle pci = create_qbject("Devices/PCIe/03_00_00\0", 0);
	uint64 configuration_space_phys_addr;
	get_qbject_property(pci, PCIE_CONFIGURATION_SPACE, (QbjectProperty*)&configuration_space_phys_addr);
	PciConfigSpace* config_space = (PciConfigSpace*)commit_pages(KHEAP, sizeof(PciConfigSpace));
	assign_committed(config_space, sizeof(PciConfigSpace), configuration_space_phys_addr);
	*((uint32*)((uint8*)config_space + 0xd4)) = 0xFEE00000;
	*((uint16*)((uint8*)config_space + 0xdc)) = NETWORK_INTERRUPT;
	*((uint32*)((uint8*)config_space + 0xd2)) = 0x1;
	screen_printf("0xd0 : 0x%x\n0xd4 : 0x%x\n0xd8 : 0x%x\n0xdc : 0x%x\n", *((uint32*)((uint8*)config_space+0xd0)), *((uint32*)((uint8*)config_space + 0xd4)), *((uint32*)((uint8*)config_space + 0xd8)), *((uint32*)((uint8*)config_space + 0xdc)));
	screen_printf("BAR0=0x%x\nBAR1=0x%x\nBAR2=0x%x\nBAR3=0x%x\n", config_space->type0_header.bar0, config_space->type0_header.bar1, config_space->type0_header.bar2, config_space->type0_header.bar3);
	uint8* memoryBAR = (uint8*)commit_pages(KHEAP, 0x8000);
	Gtemp = memoryBAR;
	uint64 temp2 = config_space->type0_header.bar0;
	assign_committed(memoryBAR, 0x8000, temp2);
	screen_printf("address low: %x\n", *((uint32*)(memoryBAR+0x5400)), 0, 0, 0);
	screen_printf("intel 82574 interrupt mask is: %x\n", *((uint32*)(memoryBAR+0xd0)), 0,0,0);
	uint8* descs = (uint8*)alloc_pages(KHEAP, 0x1000);
	volatile uint32 temp = (*PTE(descs))&(~3);
	*((uint32*)(memoryBAR + 0x03800)) = temp;
	*((uint32*)(memoryBAR + 0x03804)) = 0;
	*((uint32*)(memoryBAR + 0x03808)) = 0x400;
	*((uint32*)(memoryBAR + 0x03810)) = 0;
	*((uint32*)(memoryBAR + 0x03818)) = 0;
	uint8* data = alloc_pages(KHEAP, 0x1000);
	memcpy(data, "This frame was sent from Qube\0", 30);
	volatile uint64 temp3 = (*PTE(data))&(~0xFFF);
	*((uint64*)descs) = temp3;
	*((uint64*)(descs+8)) = 0x01000020;
	*((uint32*)(memoryBAR + 0x00400)) = 0xA;
	*((uint32*)(memoryBAR + 0x03818)) = 1;

	uint8* recv_descs = (uint8*)alloc_pages(KHEAP, 0x1000);
	uint8* recv_data = (uint8*)alloc_pages(KHEAP, 0x1000);
	temp = (*PTE(recv_descs))&(~3);
	*((uint32*)(memoryBAR + 0x02800)) = temp;
	*((uint32*)(memoryBAR + 0x02804)) = 0;
	*((uint32*)(memoryBAR + 0x02808)) = 0x1000;
	*((uint32*)(memoryBAR + 0x02810)) = 0;
	*((uint32*)(memoryBAR + 0x02818)) = 3;
	*((uint64*)recv_descs) = (*PTE(data))&(~0xFFF);
	*((uint64*)(recv_descs+16)) = (*PTE(data))&(~0xFFF)+0x800;
	*((uint64*)(recv_descs+32)) = (*PTE(data+0x1000))&(~0xFFF);
	*((uint64*)(recv_descs+48)) = (*PTE(data+0x1000))&(~0xFFF)+0x800;
	//temp = config_space->type0_header.interrupt_line;
	//configure_ioapic_interrupt(temp, NETWORK_INTERRUPT, DM_FIXED, 0);
	register_isr(NETWORK_INTERRUPT, (ISR) nullfunc);
	//enable_ioapic_interrupt(temp);
	*((uint32*)(memoryBAR + 0x000c8)) = 0;
	temp = *((uint32*)(memoryBAR + 0x000c4));
	screen_printf("intel 82574 itr is: %x\n", temp, 0, 0, 0);
	temp = *((uint32*)(memoryBAR + 0x000e8));
	screen_printf("intel 82574eitr is: %x\n", temp, 0, 0, 0);
	temp = *((uint32*)(memoryBAR + 0x000c0));
	screen_printf("imc before: %x\n", temp, 0, 0, 0);
	*((uint32*)(memoryBAR + 0x000d0)) = 0x00084; // enable interrupt RXDMT 
	*((uint32*)(memoryBAR + 0x02c00)) = 0;
	//*((uint32*)(memoryBAR + 0x000c8)) = 0xffffffff; // enable interrupt RXDMT 
	*((uint32*)(memoryBAR + 0x00100)) = 0x202; // enable receive packets.
	screen_printf("rctl is: %x\n", *((uint32*)(memoryBAR + 0x00100)), 0, 0, 0);
	screen_printf("before: ", 0, 0, 0, 0);
	for (uint32 ii = 0x4000; ii < 0x4100; ii+=4) {
		screen_printf("%x, ", *((uint32*)(memoryBAR + ii)), 0, 0, 0);
	}
	screen_printf("\nbefore: ", 0, 0, 0, 0);
	for (uint32 ii = 0x4000; ii < 0x4104; ii += 4) {
		screen_printf("%x, ", *((uint32*)(memoryBAR + ii)), 0, 0, 0);
	}
	for (temp = 0;temp < 100000000;temp += 2) {
		for (temp3 = 0;temp3 < 100000000;temp3 += 2) {
			temp3 -= 1;
		}
		temp -= 1;
	}
	screen_printf("\nafter:  ", 0, 0, 0, 0);
	for (uint32 ii = 0x4000; ii < 0x4104; ii+=4) {
		screen_printf("%x, ", *((uint32*)(memoryBAR + ii)), 0, 0, 0);
	}
	temp = *((uint32*)(memoryBAR + 0x000c0));
	
	screen_printf("imc after: %x\n", temp, 0, 0, 0);
	screen_printf("intel 82574 interrupt mask is: %x\n", *((uint32*)(memoryBAR + 0xd0)), 0, 0, 0);

	return QSuccess;
	*/
}
