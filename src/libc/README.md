# C Standard Library Implementation for aceOS

This directory contains a basic implementation of the C standard library for the aceOS operating system.

## Overview

The C standard library provides essential functions and utilities that C programs rely on. This implementation includes:

- Basic string manipulation (string.h)
- Standard I/O operations (stdio.h)
- Memory allocation (stdlib.h)
- Variable arguments (stdarg.h)
- Common type definitions (stddef.h)

## Implementation Details

### Headers

- **stddef.h**: Defines basic types like `size_t`, `NULL`, etc.
- **string.h**: String and memory manipulation functions
- **stdio.h**: I/O functions like `printf`, `putchar`, etc.
- **stdlib.h**: Memory allocation, random numbers, and other utilities
- **stdarg.h**: Support for variadic functions
- **libc.h**: Main header that includes all other headers

### Implementation Files

- **string.c**: Implements string functions like `strlen`, `strcpy`, etc.
- **stdio.c**: Implements I/O functions, connects to the OS's terminal functions
- **stdlib.c**: Implements memory allocation, string conversion, etc.
- **libc.c**: Contains initialization code for the C library

## Memory Management

The memory allocator uses a simple linked list of memory blocks to manage the heap. Each block has metadata that tracks its size and whether it's free or in use.

## Integration with aceOS

The library hooks into aceOS via:

- `terminal_putchar` for character output
- `keyboard_getchar` for character input

## Usage

To use the C standard library in your kernel or userspace programs:

1. Include the main header: `#include "libc/libc.h"`
2. Initialize the library by calling `libc_init()` early in your kernel's execution
3. Use standard C functions like `printf`, `malloc`, `strlen`, etc.

## Limitations

This is a minimal implementation with known limitations:

- Limited file operations (focused on console I/O)
- Basic memory management without advanced features
- Simplified implementations of formatted I/O functions
- No support for threading or concurrency primitives 