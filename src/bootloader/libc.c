
#include "libc.h"
#include "../Common/Qube.h"

void memcpy(char * dst, char * src, int count) {
	for (int i = 0; i < count; i++, dst++, src++) *dst = *src;
	return;
}

void memset(char *dst, char chr, int count) {
	for (int i = 0; i < count; i++, dst++) *dst = chr;
	return;
}

int memcmp(char * src, char * dst, int count) {
	for (int i = 0; i < count; i++,src++,dst++) if (*src != *dst) return *src - *dst;
	return 0;
}
int strcmp(char * src, char * dst) {
	for (;*src != NULL && *dst != NULL;src++,dst++) {
		if (*src != *dst) return *src - *dst;
	}
	return *src - *dst;
}

int strlen(char * str) {
	int i;
	for (i = 0; *str != '\x00'; str++, i++);
	return i;
}

void strcpy(char * dst, char * src) {
	while (*src) {
		*dst = *src;
		dst++;
		src++;
	}
	*dst = '\x00';
	return;
}

