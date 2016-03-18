#include "kernel_loader.h"
// This file initialize the very first things in the kernel:
// 1. init the KernelGlobalData struct.
//    a. Init the symbols table. (The symbols table will be right after 
// 2. go over a list of blobs that contains the stage1-init-chain-drivers and load them.
//
//
// The file is compiled as image and it found in the memory right before the KernelGlobalData struct.


void _start(KernelGlobalData * kgd) {
    STAGE0Stage1DriversList * driver = g_stage1_drivers;
    
    // First thing, zeroing the kgd struct:
    // TODO: more efficient.
    for (int i = 0 ; i < sizeof(KernelGlobalData) ; i++) ((char *)kgd)[i] = 0;
    
    // Init the allocator:
    kgd->STAGE0_next_mem = ((void *)kgd) + sizeof(KernelGlobalData);
    
    // Init the symbol table:
    next_mem = STAGE0_init_symbols(kgd);
    
    // Now load the drivers one by one:
    do {
        STAGE0_load_eff(kgd, (Eff *) driver->data);
        driver += driver->size;
    } while (driver.size != 0);
    // The last driver should not return.
    // NEVER REACH HERE.
}

void * STAGE0_malloc(KernelGlobalData * kgd, unsigned int size) {
    size = (size + ALLOCATOR_ALIGN) & (1<<ALLOCATOR_ALIGN);
    void * ret = kgd->next_mem;
    kgd->next_mem += size;
    return ret;
}

// The function init the symbols table.
// In the STAGE0 we support just one list of symbols at size of NUMBER_OF_SYMBOLS_IN_THE_FIRST_LIST.
// If we overflow this, we will stuck.
// The function returns the next available memory.
void * STAGE0_init_symbols(KernelGlobalData * kgd) {
    kgd->syms = STAGE0_malloc(kgd, sizeof(Symbols) + sizeof(Symbol) * NUMBER_OF_SYMBOLS_IN_LIST);
    kgd->next = NULL;
    kgd->num_of_symbols = 0;
}

void STAGE0_add_symbol(KernelGlobalData * kgd, uchar * name, void * address) {
    gword nos = kgd->syms->num_of_symbols;
    struct Symbol s = NULL;
    uint name_size = 0;
    
    if (nos >= NUMBER_OF_SYMBOLS_IN_LIST) { // We support just one list for now.
        STAGE0_suicide();
    }
    s = kgd->syms.kgd->symbols[nos];
    s.address = address;
    name_size = STAGE0_strlen(name);
    s.name = STAGE0_malloc(name_size);
    STAGE0_strcpy(s.name, name);
    return;
}


void * STAGE0_load_eff(KernelGlobalData * kgd, Eff * eff) {
    int i;
    EffSegment * seg;
    EffRelocation * rel;
    void * ret;
    // Sanity
    if (magic != EFF_MAGIC) {
        STAGE0_suicide();
    }
    
    // Alloc space
    void * ret = STAGE0_malloc(kgd, eff->span_of_memory);
    
    // Copy segments to the memory
    seg = eff + eff->segments_offset;
    for (i = 0 ; i < eff->num_of_segments ; i++) {
        if (seg->type & EffSegmentType_BYTES) {
            STAGE0_memcpy(ret + seg->virtual_address, eff + seg->offset_in_file, seg->size);
        } else if (seg->type & EffSegmentType_BSS) {
            STAGE0_memset(ret + seg->virtual_address, '\x00', seg->size);
        } else {
            STAGE0_suicide();
        }
        seg += sizeof(EffSegment);
    }
    
    // Relocating
    rel = eff + eff->relocations_offset;
    for (i = 0 ; i < eff->num_of_relocations ; i++) {
        *(ret + rel->virtual_address_offset) += ret;
    }
    
    // Exports:
    exp = eff + eff->exports_offset;
    for (i = 0 ; i < eff->num_of_exports ; i++) {
        STAGE0_add_symbol(kgd, exp->name_offset, ret + exp->virtual_address_offset);
    }
    
    // Imports:
    imp = eff + eff->imports_offset;
    for (i = 0 ; i < eff->num_of_imports ; i++) {
        if (*(ret + imp->virtual_address_offset) = STAGE0_find_symbol(kgd, imp->name_offset) == NULL) {
            STAGE0_suicde();
        }
    }
    
    return;
}

void STAGE0_suicide() {
    void * s = 0;
    *s = 0;
}
STAGE0Stage1DriversList g_stage1_drivers[0];




