#include "../include/process.h"
#include "../include/memory.h"
#include "../include/libc/string.h"
#include "../include/serial.h"
#include "../include/utils.h"

// Scheduler state
static task_queue_t ready_queue;
static scheduler_stats_t stats;
static int scheduler_enabled = 0;

// External references
extern process_t* current_process;
extern uint32_t timer_ticks;

// Initialize scheduler
void scheduler_init(void) {
    serial_write_string("Initializing scheduler...\n");
    
    // Initialize ready queue
    ready_queue.head = NULL;
    ready_queue.tail = NULL;
    ready_queue.count = 0;
    
    // Initialize statistics
    memset(&stats, 0, sizeof(scheduler_stats_t));
    
    scheduler_enabled = 1;
    
    serial_write_string("Scheduler initialized\n");
}

// Add process to ready queue
void scheduler_add_process(process_t* process) {
    if (!process || !scheduler_enabled) {
        return;
    }
    
    process->state = PROCESS_STATE_READY;
    process->next = NULL;
    
    if (ready_queue.tail) {
        ready_queue.tail->next = process;
        ready_queue.tail = process;
    } else {
        ready_queue.head = ready_queue.tail = process;
    }
    
    ready_queue.count++;
    stats.total_processes++;
    
    serial_write_string("Added process to ready queue: ");
    serial_write_string(process->name);
    serial_write_string("\n");
}

// Remove process from ready queue
void scheduler_remove_process(process_t* process) {
    if (!process || !scheduler_enabled || ready_queue.count == 0) {
        return;
    }
    
    process_t* current = ready_queue.head;
    process_t* previous = NULL;
    
    // Find and remove the process
    while (current) {
        if (current == process) {
            if (previous) {
                previous->next = current->next;
            } else {
                ready_queue.head = current->next;
            }
            
            if (current == ready_queue.tail) {
                ready_queue.tail = previous;
            }
            
            ready_queue.count--;
            current->next = NULL;
            break;
        }
        
        previous = current;
        current = current->next;
    }
}

// Get next process from ready queue (round-robin with priority)
static process_t* get_next_process(void) {
    if (ready_queue.count == 0) {
        return NULL;
    }
    
    // Simple round-robin for now
    // TODO: Implement priority-based scheduling
    process_t* next_process = ready_queue.head;
    
    if (next_process) {
        ready_queue.head = next_process->next;
        if (ready_queue.head == NULL) {
            ready_queue.tail = NULL;
        }
        ready_queue.count--;
        next_process->next = NULL;
    }
    
    return next_process;
}

// Schedule next process
void scheduler_schedule(void) {
    if (!scheduler_enabled) {
        return;
    }
    
    process_t* next_process = get_next_process();
    
    if (!next_process) {
        // No processes to run, continue with current or idle
        if (current_process && current_process->state == PROCESS_STATE_RUNNING) {
            return;
        }
        
        // Enter idle state
        stats.idle_time++;
        return;
    }
    
    process_t* previous_process = current_process;
    
    // Save current process context if it's still running
    if (previous_process && previous_process->state == PROCESS_STATE_RUNNING) {
        save_context(previous_process);
        previous_process->state = PROCESS_STATE_READY;
        
        // Add back to ready queue if not terminated
        if (previous_process->state != PROCESS_STATE_TERMINATED) {
            scheduler_add_process(previous_process);
        }
    }
    
    // Switch to next process
    current_process = next_process;
    next_process->state = PROCESS_STATE_RUNNING;
    next_process->time_used = 0;
    
    // Update statistics
    stats.context_switches++;
    stats.running_processes = ready_queue.count + 1;
    
    // Load context
    load_context(next_process);
    
    // Switch virtual memory if needed
    if (next_process->page_directory) {
        vmm_switch_page_directory(next_process->page_directory);
    }
    
    serial_write_string("Scheduled process: ");
    serial_write_string(next_process->name);
    serial_write_string("\n");
}

// Timer tick for preemptive scheduling
void scheduler_tick(void) {
    if (!scheduler_enabled || !current_process) {
        return;
    }
    
    stats.time_slices++;
    current_process->time_used++;
    current_process->total_time++;
    
    // Check if time slice expired
    if (current_process->time_used >= current_process->time_slice) {
        scheduler_yield();
    }
}

// Yield CPU to next process
void scheduler_yield(void) {
    if (!scheduler_enabled) {
        return;
    }
    
    scheduler_schedule();
}

// Print scheduler statistics
void scheduler_print_stats(void) {
    char buffer[32];
    
    serial_write_string("\n=== SCHEDULER STATISTICS ===\n");
    
    serial_write_string("Total processes created: ");
    itoa(stats.total_processes, buffer, 10);
    serial_write_string(buffer);
    serial_write_string("\n");
    
    serial_write_string("Running processes: ");
    itoa(stats.running_processes, buffer, 10);
    serial_write_string(buffer);
    serial_write_string("\n");
    
    serial_write_string("Ready queue count: ");
    itoa(ready_queue.count, buffer, 10);
    serial_write_string(buffer);
    serial_write_string("\n");
    
    serial_write_string("Context switches: ");
    itoa(stats.context_switches, buffer, 10);
    serial_write_string(buffer);
    serial_write_string("\n");
    
    serial_write_string("Time slices: ");
    itoa(stats.time_slices, buffer, 10);
    serial_write_string(buffer);
    serial_write_string("\n");
    
    serial_write_string("Idle time: ");
    itoa(stats.idle_time, buffer, 10);
    serial_write_string(buffer);
    serial_write_string("\n");
    
    if (current_process) {
        serial_write_string("Current process: ");
        serial_write_string(current_process->name);
        serial_write_string(" (PID ");
        itoa(current_process->pid, buffer, 10);
        serial_write_string(buffer);
        serial_write_string(")\n");
    }
    
    serial_write_string("============================\n");
}

// Context switching functions (simplified for now)
void save_context(process_t* process) {
    if (!process) return;
    
    // Simplified context saving - just save general purpose registers
    // In a real implementation, this would be done in the interrupt handler
    process->eax = 0;  // These would be saved during interrupt
    process->ebx = 0;
    process->ecx = 0;
    process->edx = 0;
    process->esi = 0;
    process->edi = 0;
    process->esp = 0;  // Would be saved from current stack
    process->ebp = 0;
    process->eflags = 0x202; // Default flags with interrupts enabled
}

void load_context(process_t* process) {
    if (!process) return;
    
    // Simplified context loading - in real implementation this would
    // restore registers and jump to the process
    // For now, just mark that we've "loaded" the context
    process->state = PROCESS_STATE_RUNNING;
} 