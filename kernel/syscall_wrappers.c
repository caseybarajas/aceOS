#include "syscall.h"
#include <stdint.h>

// Helper macro for system calls with no arguments
#define SYSCALL0(name, number) \
int32_t sys_##name(void) { \
    int32_t result; \
    asm volatile ("int $0x80" \
                  : "=a" (result) \
                  : "a" (number) \
                  : "memory"); \
    return result; \
}

// Helper macro for system calls with one argument
#define SYSCALL1(name, number, type1) \
int32_t sys_##name(type1 arg1) { \
    int32_t result; \
    asm volatile ("int $0x80" \
                  : "=a" (result) \
                  : "a" (number), "b" (arg1) \
                  : "memory"); \
    return result; \
}

// Helper macro for system calls with two arguments
#define SYSCALL2(name, number, type1, type2) \
int32_t sys_##name(type1 arg1, type2 arg2) { \
    int32_t result; \
    asm volatile ("int $0x80" \
                  : "=a" (result) \
                  : "a" (number), "b" (arg1), "c" (arg2) \
                  : "memory"); \
    return result; \
}

// Helper macro for system calls with three arguments
#define SYSCALL3(name, number, type1, type2, type3) \
int32_t sys_##name(type1 arg1, type2 arg2, type3 arg3) { \
    int32_t result; \
    asm volatile ("int $0x80" \
                  : "=a" (result) \
                  : "a" (number), "b" (arg1), "c" (arg2), "d" (arg3) \
                  : "memory"); \
    return result; \
}

// Helper macro for system calls with four arguments
#define SYSCALL4(name, number, type1, type2, type3, type4) \
int32_t sys_##name(type1 arg1, type2 arg2, type3 arg3, type4 arg4) { \
    int32_t result; \
    asm volatile ("int $0x80" \
                  : "=a" (result) \
                  : "a" (number), "b" (arg1), "c" (arg2), "d" (arg3), "S" (arg4) \
                  : "memory"); \
    return result; \
}

// Helper macro for system calls with five arguments
#define SYSCALL5(name, number, type1, type2, type3, type4, type5) \
int32_t sys_##name(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5) { \
    int32_t result; \
    asm volatile ("int $0x80" \
                  : "=a" (result) \
                  : "a" (number), "b" (arg1), "c" (arg2), "d" (arg3), "S" (arg4), "D" (arg5) \
                  : "memory"); \
    return result; \
}

// Special wrapper for malloc that returns void*
void* sys_malloc(uint32_t size) {
    void* result;
    asm volatile ("int $0x80"
                  : "=a" (result)
                  : "a" (SYS_MALLOC), "b" (size)
                  : "memory");
    return result;
}

// Generate system call wrappers using the macros
SYSCALL1(exit, SYS_EXIT, int32_t)
SYSCALL3(read, SYS_READ, int32_t, void*, uint32_t)
SYSCALL3(write, SYS_WRITE, int32_t, const void*, uint32_t)
SYSCALL2(open, SYS_OPEN, const char*, int32_t)
SYSCALL1(close, SYS_CLOSE, int32_t)
SYSCALL1(free, SYS_FREE, void*)
SYSCALL0(getpid, SYS_GETPID)
SYSCALL1(sleep, SYS_SLEEP, uint32_t)
SYSCALL1(chdir, SYS_CHDIR, const char*)
SYSCALL2(getcwd, SYS_GETCWD, char*, uint32_t)
SYSCALL1(mkdir, SYS_MKDIR, const char*)
SYSCALL1(rmdir, SYS_RMDIR, const char*)
SYSCALL1(unlink, SYS_UNLINK, const char*)
SYSCALL2(stat, SYS_STAT, const char*, void*)
SYSCALL0(time, SYS_TIME)

// Test function to demonstrate system call usage
void test_system_calls(void) {
    // Test getpid
    int32_t pid = sys_getpid();
    
    // Test write to stdout
    const char* msg = "Hello from system call!\n";
    sys_write(STDOUT_FILENO, msg, 25);
    
    // Test malloc and free
    void* ptr = sys_malloc(1024);
    if (ptr) {
        sys_free(ptr);
    }
    
    // Test time
    uint32_t current_time = sys_time();
    
    // Test getcwd
    char cwd[256];
    sys_getcwd(cwd, sizeof(cwd));
    
    // Sleep test removed to prevent hanging the kernel
    // (sleep would block keyboard input in single-threaded kernel)
} 