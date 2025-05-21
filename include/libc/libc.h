#ifndef LIBC_H
#define LIBC_H

// Include all standard library headers
#include "libc/stddef.h"
#include "libc/stdint.h"
#include "libc/string.h"
#include "libc/stdio.h"
#include "libc/stdlib.h"
#include "libc/stdarg.h"

// Library initialization function
void libc_init(void);

#endif /* LIBC_H */ 