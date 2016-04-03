#ifndef __BOOTLOADER_LIBC_H__
#define __BOOTLOADER_LIBC_H__

void memcpy(char * dst, char * src, int count);
void memset(char *dst, char chr, int count);
int memcmp(char * src, char * dst, int count);
int strcmp(char * srt, char * dst);
void strcpy(char * dst, char * src);
int strlen(char * str);
#endif // __BOOTLOADER_LIBC_H__
