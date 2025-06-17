#include "../include/process.h"
#include "../include/memory.h"
#include "../include/libc/string.h"
#include "../include/serial.h"
#include "../include/utils.h"

// Global process table
static process_t process_table[MAX_PROCESSES];
process_t* current_process = NULL;  // Remove static to make it globally accessible
static uint32_t next_pid = 1;
static int multitasking_enabled = 0;

// Timer for preemptive scheduling
extern uint32_t timer_ticks;

// Initialize process management
void process_init(void) {
    serial_write_string("Initializing process management...\n");
    
    // Clear process table
    memset(process_table, 0, sizeof(process_table));
    
    // Create kernel process (PID 0)
    process_t* kernel_process = &process_table[0];
    kernel_process->pid = 0;
    kernel_process->parent_pid = 0;
    strcpy(kernel_process->name, "kernel");
    kernel_process->state = PROCESS_STATE_RUNNING;
    kernel_process->priority = PROCESS_PRIORITY_HIGH;
    kernel_process->time_slice = 100; // 100ms
    kernel_process->page_directory = NULL; // Uses kernel page directory
    strcpy(kernel_process->current_directory, "/");
    
    current_process = kernel_process;
    
    serial_write_string("Process management initialized\n");
}

// Get next available PID
uint32_t process_get_next_pid(void) {
    uint32_t pid = next_pid++;
    
    // Wrap around if necessary (avoid PID 0 which is kernel)
    if (next_pid >= MAX_PROCESSES) {
        next_pid = 1;
    }
    
    return pid;
}

// Create a new process
process_t* process_create(const char* name, void* entry_point, uint32_t priority) {
    // Find free slot in process table
    process_t* process = NULL;
    for (int i = 1; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == 0) { // Unused slot
            process = &process_table[i];
            break;
        }
    }
    
    if (!process) {
        serial_write_string("ERROR: No free process slots\n");
        return NULL;
    }
    
    // Initialize process
    memset(process, 0, sizeof(process_t));
    process->pid = process_get_next_pid();
    process->parent_pid = current_process ? current_process->pid : 0;
    strncpy(process->name, name, 31);
    process->name[31] = '\0';
    process->state = PROCESS_STATE_READY;
    process->priority = priority;
    process->time_slice = (priority == PROCESS_PRIORITY_HIGH) ? 50 : 
                         (priority == PROCESS_PRIORITY_NORMAL) ? 100 : 200;
    
    // Set up memory space
    process->page_directory = vmm_create_page_directory();
    if (!process->page_directory) {
        serial_write_string("ERROR: Failed to create page directory\n");
        return NULL;
    }
    
    // Allocate kernel stack
    process->kernel_stack = pmm_alloc_frame();
    if (!process->kernel_stack) {
        serial_write_string("ERROR: Failed to allocate kernel stack\n");
        return NULL;
    }
    
    // Allocate user stack
    process->user_stack = pmm_alloc_frame();
    if (!process->user_stack) {
        serial_write_string("ERROR: Failed to allocate user stack\n");
        pmm_free_frame(process->kernel_stack);
        return NULL;
    }
    
    // Map user stack in virtual memory
    vmm_map_page(process->page_directory, USER_VIRTUAL_BASE + 0x10000, 
                 process->user_stack, PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);
    
    // Set up initial context
    process->eip = (uint32_t)entry_point;
    process->esp = USER_VIRTUAL_BASE + 0x10000 + PROCESS_STACK_SIZE - 4;
    process->ebp = process->esp;
    process->eflags = 0x202; // Enable interrupts
    
    // Set up heap
    process->heap_start = USER_VIRTUAL_BASE + 0x20000;
    process->heap_end = USER_VIRTUAL_BASE + 0x100000; // 1MB heap
    
    // Copy current directory from parent
    if (current_process) {
        strcpy(process->current_directory, current_process->current_directory);
    } else {
        strcpy(process->current_directory, "/");
    }
    
    process->creation_time = timer_ticks;
    
    serial_write_string("Created process: ");
    serial_write_string(name);
    serial_write_string(" (PID ");
    char buffer[16];
    itoa(process->pid, buffer, 10);
    serial_write_string(buffer);
    serial_write_string(")\n");
    
    return process;
}

// Destroy a process
void process_destroy(process_t* process) {
    if (!process || process->pid == 0) {
        return; // Can't destroy kernel process
    }
    
    serial_write_string("Destroying process: ");
    serial_write_string(process->name);
    serial_write_string("\n");
    
    // Free memory
    if (process->kernel_stack) {
        pmm_free_frame(process->kernel_stack);
    }
    if (process->user_stack) {
        pmm_free_frame(process->user_stack);
    }
    
    // TODO: Free all pages in process page directory
    
    // Mark as terminated
    process->state = PROCESS_STATE_TERMINATED;
    
    // Clear the process slot after a delay to allow cleanup
    memset(process, 0, sizeof(process_t));
}

// Get current running process
process_t* process_get_current(void) {
    return current_process;
}

// Get process by PID
process_t* process_get_by_pid(uint32_t pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == pid && process_table[i].state != 0) {
            return &process_table[i];
        }
    }
    return NULL;
}

// Exit current process
void process_exit(int exit_code) {
    if (!current_process || current_process->pid == 0) {
        return; // Can't exit kernel process
    }
    
    current_process->exit_code = exit_code;
    current_process->state = PROCESS_STATE_TERMINATED;
    
    serial_write_string("Process ");
    serial_write_string(current_process->name);
    serial_write_string(" exited with code ");
    char buffer[16];
    itoa(exit_code, buffer, 10);
    serial_write_string(buffer);
    serial_write_string("\n");
    
    // Schedule next process
    scheduler_schedule();
}

// Enable multitasking
void enable_multitasking(void) {
    multitasking_enabled = 1;
    serial_write_string("Multitasking enabled\n");
}

// Disable multitasking
void disable_multitasking(void) {
    multitasking_enabled = 0;
    serial_write_string("Multitasking disabled\n");
}

// Check if multitasking is enabled
int is_multitasking_enabled(void) {
    return multitasking_enabled;
}

// Simple test process function
void test_process1(void) {
    while (1) {
        serial_write_string("Test Process 1 running\n");
        // Simple delay
        for (volatile int i = 0; i < 1000000; i++);
    }
}

// Simple test process function
void test_process2(void) {
    while (1) {
        serial_write_string("Test Process 2 running\n");
        // Simple delay
        for (volatile int i = 0; i < 1000000; i++);
    }
} 