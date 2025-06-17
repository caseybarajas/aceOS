#include "syscall.h"
#include "isr.h"
#include "idt.h"
#include "fs.h"
#include "process.h"
#include "timer.h"
#include "serial.h"
#include "memory.h"
#include "libc/libc.h"
#include <stdint.h>

// Current errno value (per-process in a real OS)
static uint32_t current_errno = 0;

// System call statistics
static uint32_t syscall_count[SYS_MAX] = {0};
static uint32_t total_syscalls = 0;

// Initialize system call interface
void syscall_init(void) {
    // Register system call interrupt handler (interrupt 0x80 = 128)
    idt_set_gate(128, (uint32_t)isr128, 0x08, 0xEE);  // 0xEE = user-mode accessible
    register_interrupt_handler(128, syscall_handler);
    
    serial_write_string("System call interface initialized (INT 0x80)\n");
}

// Main system call dispatcher
void syscall_handler(registers_t regs) {
    // System call number is in EAX
    uint32_t syscall_no = regs.eax;
    
    // Arguments are in EBX, ECX, EDX, ESI, EDI
    uint32_t arg1 = regs.ebx;
    uint32_t arg2 = regs.ecx;
    uint32_t arg3 = regs.edx;
    uint32_t arg4 = regs.esi;
    uint32_t arg5 = regs.edi;
    
    // Update statistics
    total_syscalls++;
    if (syscall_no < SYS_MAX) {
        syscall_count[syscall_no]++;
    }
    
    // Debug output
    serial_write_string("SYSCALL: ");
    serial_write_dec(syscall_no);
    serial_write_string(" args: ");
    serial_write_dec(arg1);
    serial_write_string(", ");
    serial_write_dec(arg2);
    serial_write_string(", ");
    serial_write_dec(arg3);
    serial_write_string("\n");
    
    // Reset errno
    current_errno = 0;
    int32_t result = -1;
    
    // Dispatch to appropriate handler
    switch (syscall_no) {
        case SYS_EXIT:
            result = kernel_exit((int32_t)arg1);
            break;
            
        case SYS_READ:
            result = kernel_read((int32_t)arg1, (void*)arg2, arg3);
            break;
            
        case SYS_WRITE:
            result = kernel_write((int32_t)arg1, (const void*)arg2, arg3);
            break;
            
        case SYS_OPEN:
            result = kernel_open((const char*)arg1, (int32_t)arg2);
            break;
            
        case SYS_CLOSE:
            result = kernel_close((int32_t)arg1);
            break;
            
        case SYS_MALLOC:
            result = (int32_t)kernel_malloc(arg1);
            break;
            
        case SYS_FREE:
            result = kernel_free((void*)arg1);
            break;
            
        case SYS_GETPID:
            result = kernel_getpid();
            break;
            
        case SYS_SLEEP:
            result = kernel_sleep(arg1);
            break;
            
        case SYS_CHDIR:
            result = kernel_chdir((const char*)arg1);
            break;
            
        case SYS_GETCWD:
            result = kernel_getcwd((char*)arg1, arg2);
            break;
            
        case SYS_MKDIR:
            result = kernel_mkdir((const char*)arg1);
            break;
            
        case SYS_RMDIR:
            result = kernel_rmdir((const char*)arg1);
            break;
            
        case SYS_UNLINK:
            result = kernel_unlink((const char*)arg1);
            break;
            
        case SYS_STAT:
            result = kernel_stat((const char*)arg1, (void*)arg2);
            break;
            
        case SYS_TIME:
            result = (int32_t)kernel_time();
            break;
            
        default:
            current_errno = EINVAL;  // Invalid system call
            result = -1;
            serial_write_string("SYSCALL: Invalid system call number\n");
            break;
    }
    
    // Return result in EAX
    regs.eax = (uint32_t)result;
    
    // Debug output
    serial_write_string("SYSCALL result: ");
    serial_write_dec(result);
    serial_write_string("\n");
}

// System call implementations

int32_t kernel_exit(int32_t status) {
    serial_write_string("Process exit with status: ");
    serial_write_dec(status);
    serial_write_string("\n");
    
    // In a real OS, this would terminate the current process
    // For now, we'll just return the status
    return status;
}

int32_t kernel_read(int32_t fd, void* buffer, uint32_t count) {
    if (!buffer) {
        current_errno = EFAULT;
        return -1;
    }
    
    if (fd == STDIN_FILENO) {
        // Read from standard input (keyboard)
        // For now, return empty read
        current_errno = EAGAIN;
        return 0;
    } else {
        // File descriptor-based read would go here
        // For now, just return error
        current_errno = EBADF;
        return -1;
    }
}

int32_t kernel_write(int32_t fd, const void* buffer, uint32_t count) {
    if (!buffer) {
        current_errno = EFAULT;
        return -1;
    }
    
    if (fd == STDOUT_FILENO || fd == STDERR_FILENO) {
        // Write to standard output
        const char* str = (const char*)buffer;
        uint32_t written = 0;
        
        for (uint32_t i = 0; i < count && str[i] != '\0'; i++) {
            // Output to screen (would need screen output function)
            // For now, output to serial
            serial_write(str[i]);
            written++;
        }
        
        return (int32_t)written;
    } else {
        // File descriptor-based write would go here
        current_errno = EBADF;
        return -1;
    }
}

int32_t kernel_open(const char* pathname, int32_t flags) {
    if (!pathname) {
        current_errno = EFAULT;
        return -1;
    }
    
    // Check if file exists using filesystem
    if (fs_find(pathname) >= 0) {
        // File exists, return a dummy file descriptor
        // In a real OS, this would allocate a real fd
        return 3;  // Start after stdin/stdout/stderr
    } else {
        current_errno = ENOENT;
        return -1;
    }
}

int32_t kernel_close(int32_t fd) {
    if (fd < 0) {
        current_errno = EBADF;
        return -1;
    }
    
    if (fd <= 2) {
        // Can't close stdin/stdout/stderr
        current_errno = EBADF;
        return -1;
    }
    
    // Close file descriptor
    return 0;
}

void* kernel_malloc(uint32_t size) {
    if (size == 0) {
        current_errno = EINVAL;
        return NULL;
    }
    
    void* ptr = heap_malloc(size);
    if (!ptr) {
        current_errno = ENOMEM;
    }
    
    return ptr;
}

int32_t kernel_free(void* ptr) {
    if (!ptr) {
        current_errno = EINVAL;
        return -1;
    }
    
    heap_free(ptr);
    return 0;
}

int32_t kernel_getpid(void) {
    // Return current process ID
    // For now, return a dummy PID
    return 1;
}

int32_t kernel_sleep(uint32_t seconds) {
    if (seconds == 0) {
        return 0;
    }
    
    // Get current time
    uint32_t start_time = timer_get_ticks();
    uint32_t target_time = start_time + (seconds * 1000);  // 1000 Hz timer
    
    // Busy wait (in a real OS, this would yield to other processes)
    while (timer_get_ticks() < target_time) {
        asm("hlt");  // Halt until next interrupt
    }
    
    return 0;
}

int32_t kernel_chdir(const char* path) {
    if (!path) {
        current_errno = EFAULT;
        return -1;
    }
    
    int result = fs_change_dir(path);
    if (result != 0) {
        current_errno = ENOENT;
        return -1;
    }
    
    return 0;
}

int32_t kernel_getcwd(char* buffer, uint32_t size) {
    if (!buffer || size == 0) {
        current_errno = EFAULT;
        return -1;
    }
    
    char* current_dir = fs_get_current_dir();
    uint32_t len = strlen(current_dir);
    
    if (len >= size) {
        current_errno = ERANGE;
        return -1;
    }
    
    strncpy(buffer, current_dir, size);
    buffer[size - 1] = '\0';
    
    return 0;
}

int32_t kernel_mkdir(const char* pathname) {
    if (!pathname) {
        current_errno = EFAULT;
        return -1;
    }
    
    int result = fs_mkdir(pathname);
    if (result != 0) {
        current_errno = EEXIST;  // Or other appropriate error
        return -1;
    }
    
    return 0;
}

int32_t kernel_rmdir(const char* pathname) {
    if (!pathname) {
        current_errno = EFAULT;
        return -1;
    }
    
    int result = fs_delete(pathname);
    if (result != 0) {
        current_errno = ENOENT;
        return -1;
    }
    
    return 0;
}

int32_t kernel_unlink(const char* pathname) {
    if (!pathname) {
        current_errno = EFAULT;
        return -1;
    }
    
    int result = fs_delete(pathname);
    if (result != 0) {
        current_errno = ENOENT;
        return -1;
    }
    
    return 0;
}

int32_t kernel_stat(const char* pathname, void* statbuf) {
    if (!pathname || !statbuf) {
        current_errno = EFAULT;
        return -1;
    }
    
    // Use filesystem's fs_stat function
    fs_entry_t info;
    int result = fs_stat(pathname, &info);
    
    if (result != 0) {
        current_errno = ENOENT;
        return -1;
    }
    
    // Copy relevant information to statbuf
    // In a real OS, this would use a proper stat structure
    memcpy(statbuf, &info, sizeof(fs_entry_t));
    
    return 0;
}

uint32_t kernel_time(void) {
    // Return time in seconds since boot
    return timer_get_ticks() / 1000;  // Convert from ms to seconds
}

// Utility function to get current errno
uint32_t get_errno(void) {
    return current_errno;
}

// Print system call statistics
void syscall_print_stats(void) {
    serial_write_string("\n=== SYSTEM CALL STATISTICS ===\n");
    serial_write_string("Total system calls: ");
    serial_write_dec(total_syscalls);
    serial_write_string("\n");
    
    const char* syscall_names[] = {
        "exit", "read", "write", "open", "close", "malloc", "free", "getpid",
        "sleep", "fork", "exec", "wait", "kill", "chdir", "getcwd", "mkdir",
        "rmdir", "unlink", "stat", "time", "sbrk", "mmap", "munmap", "getuid",
        "setuid", "signal", "ioctl", "seek", "dup", "pipe"
    };
    
    for (uint32_t i = 0; i < SYS_MAX; i++) {
        if (syscall_count[i] > 0) {
            serial_write_string("  ");
            serial_write_string(syscall_names[i]);
            serial_write_string(": ");
            serial_write_dec(syscall_count[i]);
            serial_write_string("\n");
        }
    }
    
    serial_write_string("==============================\n");
} 