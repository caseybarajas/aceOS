#ifndef LIBC_STDDEF_H
#define LIBC_STDDEF_H

#ifndef NULL
#define NULL ((void*)0)
#endif

typedef unsigned long size_t;
typedef long ptrdiff_t;
typedef unsigned int wchar_t;

#define offsetof(type, member) __builtin_offsetof(type, member)

#endif /* LIBC_STDDEF_H */ 