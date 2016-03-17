#define NULL 0

#define uchar unsigned char

// gword and ugword are plateform specific. thay can be 64bit or 32bit.
#define gword  signed long long
#define ugword unsigned long long

struct Symbol {
    void * address;
    uchar * name;
    gword type;
}

struct Symbols {
    struct Symbols * next; // pointer to the next list of symbols.
    gword num_of_symbols; // Number of symbols in this list.
    Symbol symbols[0]; // Undefined number of symbols.
}


struct KernelGlobalData {
    void * STAGE0_next_mem;
    struct Symbols * syms;
    
}