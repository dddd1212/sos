#pragma once

#define NULL 0

#define MAX_LOADED_MODULES 0x100

#define uchar unsigned char


// gword and ugword are plateform specific. thay can be 64bit or 32bit.
#define gword  signed long long
#define ugword unsigned long long
enum PAGE_ACCESS {
	PAGE_ACCESS_NONE = 0x0,
	PAGE_ACCESS_READ = 0x1,
	PAGE_ACCESS_WRITE = 0x2,
	PAGE_ACCESS_RW = 0x3,
	PAGE_ACCESS_EXEC = 0x4,
	PAGE_ACCESS_RX = 0x5,
	PAGE_ACCESS_WX = 0x6,
	PAGE_ACCESS_RWX = 0x7,
};

struct ModulesList {
	int m[MAX_LOADED_MODULES];
};

struct KernelGlobalData {
	struct ModulesList * modules;
};