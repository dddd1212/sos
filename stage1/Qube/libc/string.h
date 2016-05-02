#ifndef __STRING_H__
#define __STRING_H__

#include "../Common/Qube.h"
// Implement here functions from this list, if you need to.
// In case of warning when compile to 32bit, try change the definition of size_t to uint32.
// To check if function is built-in, change the definition and see if you get a warning.

char * strcpy(char * dst, const char * src); // GCC BUILT_IN
char * strncpy(char * dst, const char * src, size_t count); // GCC BUILT_IN
//size_t strlcpy(char *, const char *, size_t);
//size_t strscpy(char *, const char *, size_t);
char * strcat(char * destination, const char * source); // GCC BUILT_IN
char * strncat(char * destination, const char * source, size_t num); // GCC BUILT_IN
//size_t strlcat(char *, const char *, size_t);
int    strcmp(const char * str1, const char * str2); // GCC BUILT_IN
int    strncmp(const char *, const char *, size_t); // GCC BUILT_IN
//int    strcasecmp(const char *s1, const char *s2);
//int    strncasecmp(const char *s1, const char *s2, size_t n);
//char * strchr(const char *, int);
//char * strchrnul(const char *, int);
//char * strnchr(const char *, size_t, int);
//char * strrchr(const char *, int);
//char * skip_spaces(const char *);
//char * strim(char *);
//char * strstr(const char *, const char *);
//char * strnstr(const char *, const char *, size_t);
size_t strlen(const char *); // GCC BUILT_IN
//size_t strnlen(const char *, size_t);
//char * strpbrk(const char *, const char *);
//char * strsep(char **, const char *);
//size_t strspn(const char *, const char *);
//size_t strcspn(const char *, const char *);
void * memset(void *, int, size_t); // GCC BUILT_IN
void * memcpy(void *, const void *, size_t); // GCC BUILT_IN
//void * memmove(void *, const void *, size_t);
//void * memscan(void *, int, size_t);
int    memcmp(const void *, const void *, size_t); // GCC BUILT_IN
//void * memchr(const void *, int, size_t);


#endif // __STRING_H__
