#include "../include/timer.h"
#include "../include/io.h"
#include "../include/isr.h"
#include "../include/serial.h"
#include "../include/process.h"
#include "../include/utils.h"

// Global timer state
uint32_t timer_ticks = 0;
system_time_t system_time = {0, 0, 0, 0};
static timer_stats_t stats = {0, 0, 0, 0};
static uint32_t measurement_start = 0;

// Timer callbacks (simple array for now)
#define MAX_TIMER_CALLBACKS 10
static timer_callback_t timer_callbacks[MAX_TIMER_CALLBACKS];
static int callback_count = 0;

// Initialize the timer
void timer_init(void) {
    serial_write_string("Initializing system timer...\n");
    
    // Set timer frequency
    timer_set_frequency(TIMER_FREQUENCY);
    
    // Register timer interrupt handler
    register_interrupt_handler(32, (isr_t)timer_handler); // IRQ0 = interrupt 32
    
    // Clear statistics
    stats.total_ticks = 0;
    stats.interrupts_per_second = 0;
    stats.missed_ticks = 0;
    stats.scheduler_calls = 0;
    
    // Clear callbacks
    for (int i = 0; i < MAX_TIMER_CALLBACKS; i++) {
        timer_callbacks[i] = NULL;
    }
    callback_count = 0;
    
    serial_write_string("Timer initialized at ");
    char buffer[16];
    itoa(TIMER_FREQUENCY, buffer, 10);
    serial_write_string(buffer);
    serial_write_string(" Hz\n");
}

// Set timer frequency using PIT (Programmable Interval Timer)
void timer_set_frequency(uint32_t frequency) {
    // Calculate divisor for PIT
    // PIT input clock is 1193180 Hz
    uint32_t divisor = 1193180 / frequency;
    
    // Send command byte to PIT
    outb(TIMER_COMMAND_PORT, PIT_CHANNEL_0 | PIT_ACCESS_LOHIBYTE | PIT_MODE_SQUARE_WAVE);
    
    // Send frequency divisor (low byte first, then high byte)
    outb(TIMER_DATA_PORT_0, divisor & 0xFF);
    outb(TIMER_DATA_PORT_0, (divisor >> 8) & 0xFF);
}

// Timer interrupt handler
void timer_handler(registers_t regs) {
    timer_ticks++;
    stats.total_ticks++;
    
    // Update system time every 1000 ticks (1 second at 1000 Hz)
    if (timer_ticks % 1000 == 0) {
        timer_update_system_time();
        stats.interrupts_per_second = 1000; // We know it's 1000 Hz
    }
    
    // Call registered callbacks
    for (int i = 0; i < callback_count; i++) {
        if (timer_callbacks[i]) {
            timer_callbacks[i]();
        }
    }
    
    // Call scheduler tick for multitasking
    if (is_multitasking_enabled()) {
        scheduler_tick();
        stats.scheduler_calls++;
    }
}

// Get current timer ticks
uint32_t timer_get_ticks(void) {
    return timer_ticks;
}

// Sleep for specified milliseconds
void timer_sleep(uint32_t milliseconds) {
    uint32_t start_ticks = timer_ticks;
    uint32_t target_ticks = start_ticks + milliseconds;
    
    // Busy wait (not ideal, but simple)
    while (timer_ticks < target_ticks) {
        // Yield to other processes if multitasking is enabled
        if (is_multitasking_enabled()) {
            scheduler_yield();
        }
    }
}

// Update system time based on timer ticks
void timer_update_system_time(void) {
    system_time.seconds++;
    
    if (system_time.seconds >= 60) {
        system_time.seconds = 0;
        system_time.minutes++;
        
        if (system_time.minutes >= 60) {
            system_time.minutes = 0;
            system_time.hours++;
            
            if (system_time.hours >= 24) {
                system_time.hours = 0;
                system_time.days++;
            }
        }
    }
}

// Get current system time
void timer_get_system_time(system_time_t* time) {
    if (time) {
        time->seconds = system_time.seconds;
        time->minutes = system_time.minutes;
        time->hours = system_time.hours;
        time->days = system_time.days;
    }
}

// Print timer statistics
void timer_print_stats(void) {
    char buffer[32];
    
    serial_write_string("\n=== TIMER STATISTICS ===\n");
    
    serial_write_string("Total ticks: ");
    itoa(stats.total_ticks, buffer, 10);
    serial_write_string(buffer);
    serial_write_string("\n");
    
    serial_write_string("System uptime: ");
    itoa(system_time.days, buffer, 10);
    serial_write_string(buffer);
    serial_write_string("d ");
    itoa(system_time.hours, buffer, 10);
    serial_write_string(buffer);
    serial_write_string("h ");
    itoa(system_time.minutes, buffer, 10);
    serial_write_string(buffer);
    serial_write_string("m ");
    itoa(system_time.seconds, buffer, 10);
    serial_write_string(buffer);
    serial_write_string("s\n");
    
    serial_write_string("Scheduler calls: ");
    itoa(stats.scheduler_calls, buffer, 10);
    serial_write_string(buffer);
    serial_write_string("\n");
    
    serial_write_string("Registered callbacks: ");
    itoa(callback_count, buffer, 10);
    serial_write_string(buffer);
    serial_write_string("\n");
    
    serial_write_string("Frequency: ");
    itoa(TIMER_FREQUENCY, buffer, 10);
    serial_write_string(buffer);
    serial_write_string(" Hz\n");
    
    serial_write_string("========================\n");
}

// Register a timer callback
void timer_register_callback(timer_callback_t callback) {
    if (callback_count < MAX_TIMER_CALLBACKS) {
        timer_callbacks[callback_count] = callback;
        callback_count++;
        
        serial_write_string("Timer callback registered\n");
    } else {
        serial_write_string("ERROR: Maximum timer callbacks reached\n");
    }
}

// Unregister a timer callback
void timer_unregister_callback(timer_callback_t callback) {
    for (int i = 0; i < callback_count; i++) {
        if (timer_callbacks[i] == callback) {
            // Shift remaining callbacks down
            for (int j = i; j < callback_count - 1; j++) {
                timer_callbacks[j] = timer_callbacks[j + 1];
            }
            timer_callbacks[callback_count - 1] = NULL;
            callback_count--;
            
            serial_write_string("Timer callback unregistered\n");
            return;
        }
    }
    
    serial_write_string("ERROR: Timer callback not found\n");
}

// Start a high-resolution measurement
void timer_start_measurement(void) {
    measurement_start = timer_ticks;
}

// End measurement and return elapsed milliseconds
uint32_t timer_end_measurement(void) {
    return timer_ticks - measurement_start;
}

// External itoa function
extern void itoa(int value, char* str, int base); 