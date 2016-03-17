#include "kernel.h"

#define NUMBER_OF_SYMBOLS_IN_LIST 100

#define ALLOCATOR_ALIGN 0x10


struct STAGE0Stage1DriversList {
    unsigned int size;
    unsigned char data[0];
}

struct STAGE0SymbolTableStruct {
    unsigned int num_of_symbols;
    unsigned int 
}

