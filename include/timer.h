#ifndef TIMER_H
#define TIMER_H

#include "libc/stdint.h"
#include "isr.h"

// Timer constants
#define TIMER_FREQUENCY     1000    // 1000 Hz (1ms ticks)
#define TIMER_COMMAND_PORT  0x43
#define TIMER_DATA_PORT_0   0x40
#define TIMER_IRQ           0

// Timer modes
#define TIMER_MODE_ONESHOT      0x00
#define TIMER_MODE_PERIODIC     0x02
#define TIMER_MODE_SQUARE_WAVE  0x06

// PIT channel commands
#define PIT_CHANNEL_0           0x00
#define PIT_ACCESS_LOHIBYTE     0x30
#define PIT_MODE_SQUARE_WAVE    0x06

// Time structure
typedef struct {
    uint32_t seconds;
    uint32_t minutes;
    uint32_t hours;
    uint32_t days;
} system_time_t;

// Timer statistics
typedef struct {
    uint32_t total_ticks;
    uint32_t interrupts_per_second;
    uint32_t missed_ticks;
    uint32_t scheduler_calls;
} timer_stats_t;

// Global timer variables
extern uint32_t timer_ticks;
extern system_time_t system_time;

// Function prototypes
void timer_init(void);
void timer_handler(registers_t regs);
void timer_set_frequency(uint32_t frequency);
uint32_t timer_get_ticks(void);
void timer_sleep(uint32_t milliseconds);
void timer_update_system_time(void);
void timer_get_system_time(system_time_t* time);
void timer_print_stats(void);

// Callback function type for timer events
typedef void (*timer_callback_t)(void);

// Timer callback management
void timer_register_callback(timer_callback_t callback);
void timer_unregister_callback(timer_callback_t callback);

// High-resolution timing
void timer_start_measurement(void);
uint32_t timer_end_measurement(void);

#endif /* TIMER_H */ 