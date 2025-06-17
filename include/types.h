#ifndef TYPES_H
#define TYPES_H

// Prevent system stdint.h from being included
#define _STDINT_H
#define __STDINT_H
// Prevent size_t conflicts
#define _SIZE_T_DEFINED
#define __SIZE_T__

// Basic integer types
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef long long          int64_t;

// Size and pointer types
typedef unsigned long      size_t;
typedef long               ssize_t;
typedef uint32_t           uintptr_t;
typedef int32_t            intptr_t;

// Define commonly used limits
#define SIZE_MAX           ((size_t)-1)
#define UINT32_MAX         4294967295U

#endif // TYPES_H 