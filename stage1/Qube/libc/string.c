#include "string.h"

char * strcpy(char * dst, const char * src) {
	char * ret = dst;
	while (*src) {
		*dst = *src;
		dst++;
		src++;
	}
	*dst = '\x00';
	return ret;
}

char * strncpy(char * dst, const char * src, size_t count) {
	char * ret = dst;
	BOOL null = FALSE;
	for (int i = 0; i < count; i++) {
		if (null) *dst = '\x00';
		else {
			*dst = *src;
			if (!*src) null = TRUE;
		}
	}
	return ret;
}

char * strcat(char * dst, const char * src) {
	char * ret = dst;
	dst += strlen(dst);
	strcpy(dst, src);
	return ret;
}

char * strncat(char * dst, const char * src, size_t num) {
	if (strlen(src) <= num) return strcat(dst, src);
	char * ret = dst;
	dst += strlen(dst);
	memcpy(dst, src, num);
	*(dst+num) = '\x00';
	return ret;
}

int strcmp(const char * src, const char * dst) {
	for (; *src != NULL && *dst != NULL; src++, dst++) {
		if (*src != *dst) return *src - *dst;
	}
	return *src - *dst;
}

int strncmp(const char * str1, const char * str2, size_t count) {
	for (int i = 0; i < count; i++) {
		if (*str1 != *str2) return *str1 - *str2;
	}
	return 0;
}

size_t strlen(const char * str) {
	int i;
	for (i = 0; *str != '\x00'; str++, i++);
	return i;

}

void * memset(void *dst, int chr, size_t count) {
	char * ret = dst;
	char * cdst = (char*)dst;
	for (int i = 0; i < count; i++, cdst++) *cdst = chr;
	return ret;
}

void * memcpy(void * dst, const void * src, size_t count) {
	char * cdst = (char*)dst;
	char * csrc = (char*)src;
	char * ret = dst;
	for (int i = 0; i < count; i++, cdst++, csrc++) *cdst = *csrc;
	return ret;
}

int memcmp(const void * src, const void * dst, size_t count) {
	char * cdst = (char*)dst;
	char * csrc = (char*)src;
	for (int i = 0; i < count; i++, csrc++, cdst++) if (*csrc != *cdst) return *csrc - *cdst;
	return 0;
}




