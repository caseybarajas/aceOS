#ifndef LIBC_STDLIB_H
#define LIBC_STDLIB_H

#include "libc/stddef.h"

// Memory allocation
void* malloc(size_t size);
void* calloc(size_t nmemb, size_t size);
void* realloc(void* ptr, size_t size);
void free(void* ptr);

// String conversion
int atoi(const char* nptr);
long atol(const char* nptr);
double atof(const char* nptr);
long strtol(const char* nptr, char** endptr, int base);
unsigned long strtoul(const char* nptr, char** endptr, int base);
double strtod(const char* nptr, char** endptr);

// Random numbers
#define RAND_MAX 32767
int rand(void);
void srand(unsigned int seed);

// Algorithm utilities
void qsort(void* base, size_t nmemb, size_t size, int (*compar)(const void*, const void*));
void* bsearch(const void* key, const void* base, size_t nmemb, size_t size, int (*compar)(const void*, const void*));

// Types for div and ldiv
typedef struct {
    int quot;
    int rem;
} div_t;

typedef struct {
    long quot;
    long rem;
} ldiv_t;

// Integer arithmetic
int abs(int j);
long labs(long j);
div_t div(int numer, int denom);
ldiv_t ldiv(long numer, long denom);

// Exit functions
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
void exit(int status);
void abort(void);
int atexit(void (*func)(void));

#endif /* LIBC_STDLIB_H */ 