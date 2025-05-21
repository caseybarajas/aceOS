#ifndef LIBC_STDIO_H
#define LIBC_STDIO_H

#include "libc/stddef.h"
#include "libc/stdarg.h"

// Typedefs
typedef struct _FILE FILE;
#define EOF (-1)

// Standard streams
extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

// Basic I/O functions
int putchar(int c);
int getchar(void);
int puts(const char* s);
char* gets(char* s);

// Formatted I/O
int printf(const char* format, ...);
int sprintf(char* str, const char* format, ...);
int snprintf(char* str, size_t size, const char* format, ...);
int vprintf(const char* format, va_list ap);
int vsprintf(char* str, const char* format, va_list ap);
int vsnprintf(char* str, size_t size, const char* format, va_list ap);

// File operations
FILE* fopen(const char* filename, const char* mode);
int fclose(FILE* stream);
size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream);
size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream);
int fseek(FILE* stream, long offset, int whence);
long ftell(FILE* stream);
void rewind(FILE* stream);
int fgetc(FILE* stream);
char* fgets(char* s, int size, FILE* stream);
int fputc(int c, FILE* stream);
int fputs(const char* s, FILE* stream);

#endif /* LIBC_STDIO_H */ 