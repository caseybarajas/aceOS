#ifndef SYSCALL_H
#define SYSCALL_H

#include "types.h"
#include "isr.h"

// System call numbers
#define SYS_EXIT        0   // Exit process
#define SYS_READ        1   // Read from file descriptor
#define SYS_WRITE       2   // Write to file descriptor  
#define SYS_OPEN        3   // Open file
#define SYS_CLOSE       4   // Close file descriptor
#define SYS_MALLOC      5   // Allocate memory
#define SYS_FREE        6   // Free memory
#define SYS_GETPID      7   // Get process ID
#define SYS_SLEEP       8   // Sleep for specified time
#define SYS_FORK        9   // Create new process
#define SYS_EXEC        10  // Execute program
#define SYS_WAIT        11  // Wait for child process
#define SYS_KILL        12  // Send signal to process
#define SYS_CHDIR       13  // Change directory
#define SYS_GETCWD      14  // Get current working directory
#define SYS_MKDIR       15  // Create directory
#define SYS_RMDIR       16  // Remove directory
#define SYS_UNLINK      17  // Remove file
#define SYS_STAT        18  // Get file status
#define SYS_TIME        19  // Get current time
#define SYS_SBRK        20  // Change data segment size
#define SYS_MMAP        21  // Map memory
#define SYS_MUNMAP      22  // Unmap memory
#define SYS_GETUID      23  // Get user ID
#define SYS_SETUID      24  // Set user ID
#define SYS_SIGNAL      25  // Set signal handler
#define SYS_IOCTL       26  // I/O control
#define SYS_SEEK        27  // Seek in file
#define SYS_DUP         28  // Duplicate file descriptor
#define SYS_PIPE        29  // Create pipe
#define SYS_MAX         30  // Maximum system call number

// File descriptor constants
#define STDIN_FILENO    0
#define STDOUT_FILENO   1
#define STDERR_FILENO   2

// System call result type
typedef struct {
    int32_t value;      // Return value (or error code if negative)
    uint32_t errno;     // Error number (0 if successful)
} syscall_result_t;

// Error codes
#define ENOENT      2   // No such file or directory
#define ESRCH       3   // No such process
#define EINTR       4   // Interrupted system call
#define EIO         5   // I/O error
#define ENXIO       6   // No such device or address
#define E2BIG       7   // Argument list too long
#define ENOEXEC     8   // Exec format error
#define EBADF       9   // Bad file number
#define ECHILD      10  // No child processes
#define EAGAIN      11  // Try again
#define ENOMEM      12  // Out of memory
#define EACCES      13  // Permission denied
#define EFAULT      14  // Bad address
#define EBUSY       16  // Device or resource busy
#define EEXIST      17  // File exists
#define ENODEV      19  // No such device
#define ENOTDIR     20  // Not a directory
#define EISDIR      21  // Is a directory
#define EINVAL      22  // Invalid argument
#define ENFILE      23  // File table overflow
#define EMFILE      24  // Too many open files
#define ENOTTY      25  // Not a typewriter
#define EFBIG       27  // File too large
#define ENOSPC      28  // No space left on device
#define ESPIPE      29  // Illegal seek
#define EROFS       30  // Read-only file system
#define EMLINK      31  // Too many links
#define ERANGE      34  // Math result not representable

// Function prototypes
void syscall_init(void);
void syscall_handler(registers_t regs);

// User-space system call wrapper functions (for future userspace programs)
int32_t sys_exit(int32_t status);
int32_t sys_read(int32_t fd, void* buffer, uint32_t count);
int32_t sys_write(int32_t fd, const void* buffer, uint32_t count);
int32_t sys_open(const char* pathname, int32_t flags);
int32_t sys_close(int32_t fd);
void* sys_malloc(uint32_t size);
int32_t sys_free(void* ptr);
int32_t sys_getpid(void);
int32_t sys_sleep(uint32_t seconds);

// Internal kernel implementations
int32_t kernel_exit(int32_t status);
int32_t kernel_read(int32_t fd, void* buffer, uint32_t count);
int32_t kernel_write(int32_t fd, const void* buffer, uint32_t count);
int32_t kernel_open(const char* pathname, int32_t flags);
int32_t kernel_close(int32_t fd);
void* kernel_malloc(uint32_t size);
int32_t kernel_free(void* ptr);
int32_t kernel_getpid(void);
int32_t kernel_sleep(uint32_t seconds);
int32_t kernel_chdir(const char* path);
int32_t kernel_getcwd(char* buffer, uint32_t size);
int32_t kernel_mkdir(const char* pathname);
int32_t kernel_rmdir(const char* pathname);
int32_t kernel_unlink(const char* pathname);
int32_t kernel_stat(const char* pathname, void* statbuf);
uint32_t kernel_time(void);

// Additional utility functions
uint32_t get_errno(void);
void syscall_print_stats(void);
void test_system_calls(void);

#endif // SYSCALL_H 