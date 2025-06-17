#ifndef PROCESS_H
#define PROCESS_H

#include "libc/stdint.h"
#include "memory.h"

// Process states
#define PROCESS_STATE_RUNNING   1
#define PROCESS_STATE_READY     2
#define PROCESS_STATE_BLOCKED   3
#define PROCESS_STATE_TERMINATED 4

// Process priorities
#define PROCESS_PRIORITY_HIGH   1
#define PROCESS_PRIORITY_NORMAL 2
#define PROCESS_PRIORITY_LOW    3

// Maximum number of processes
#define MAX_PROCESSES 32

// Process stack size (4KB)
#define PROCESS_STACK_SIZE 4096

// Process control block (PCB)
typedef struct process {
    uint32_t pid;                    // Process ID
    uint32_t parent_pid;             // Parent process ID
    char name[32];                   // Process name
    uint32_t state;                  // Process state
    uint32_t priority;               // Process priority
    
    // CPU context
    uint32_t eax, ebx, ecx, edx;     // General purpose registers
    uint32_t esi, edi;               // Index registers
    uint32_t esp, ebp;               // Stack and base pointers
    uint32_t eip;                    // Instruction pointer
    uint32_t eflags;                 // Flags register
    
    // Memory management
    page_directory_t* page_directory; // Virtual memory space
    uint32_t kernel_stack;           // Kernel stack pointer
    uint32_t user_stack;             // User stack pointer
    uint32_t heap_start;             // Heap start address
    uint32_t heap_end;               // Heap end address
    
    // Scheduling
    uint32_t time_slice;             // Time slice in milliseconds
    uint32_t time_used;              // Time used in current slice
    uint32_t total_time;             // Total CPU time used
    
    // File system
    char current_directory[256];     // Current working directory
    
    // Linked list for scheduler
    struct process* next;
    
    // Process creation time
    uint32_t creation_time;
    
    // Exit code
    int exit_code;
} process_t;

// Task queue for scheduler
typedef struct task_queue {
    process_t* head;
    process_t* tail;
    uint32_t count;
} task_queue_t;

// Scheduler statistics
typedef struct scheduler_stats {
    uint32_t total_processes;
    uint32_t running_processes;
    uint32_t context_switches;
    uint32_t time_slices;
    uint32_t idle_time;
} scheduler_stats_t;

// Global current process variable
extern process_t* current_process;

// System call numbers are now defined in syscall.h

// Function prototypes
// Process management
void process_init(void);
process_t* process_create(const char* name, void* entry_point, uint32_t priority);
void process_destroy(process_t* process);
process_t* process_get_current(void);
process_t* process_get_by_pid(uint32_t pid);
void process_exit(int exit_code);
uint32_t process_get_next_pid(void);

// Scheduler
void scheduler_init(void);
void scheduler_add_process(process_t* process);
void scheduler_remove_process(process_t* process);
void scheduler_tick(void);
void scheduler_yield(void);
void scheduler_schedule(void);
void scheduler_print_stats(void);

// Context switching
void context_switch(process_t* from, process_t* to);
void save_context(process_t* process);
void load_context(process_t* process);

// System calls are now handled in syscall.h

// User mode support
void enter_user_mode(uint32_t entry_point, uint32_t stack_pointer);
void setup_user_mode(void);

// Multitasking utilities
void enable_multitasking(void);
void disable_multitasking(void);
int is_multitasking_enabled(void);

// Inter-process communication (basic)
int process_send_message(uint32_t target_pid, void* message, size_t size);
int process_receive_message(void* buffer, size_t buffer_size);

#endif /* PROCESS_H */ 