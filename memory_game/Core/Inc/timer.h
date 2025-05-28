#ifndef TIMER_MOD_H
#define TIMER_MOD_H

#include <stdint.h>
#include "stm32f303xc.h"

typedef struct {
	TIM_TypeDef* ptr;
	void (*callback)();
} Timer;

void timer_rcc_init();

// Create a timer
Timer timer_init(TIM_TypeDef* timer_ptr,
				 void (*callback)(),
				 IRQn_Type timer_number,
				 int priority);

// Starts a timer
void timer_enable(Timer* timer);

// Stops a timer
void timer_disable(Timer* timer);

// Resets and starts a timer
void timer_restart(Timer* timer);

// Use oneshot mode
void timer_enable_oneshot(Timer* timer);

// Count down from ARR value to 0
void timer_set_downcount(Timer* timer);

// Count up to ARR value from 0
void timer_set_upcount(Timer* timer);

// Get timer count
uint32_t timer_get_count(Timer* timer);

// Set timer count
void timer_set_count(Timer* timer, uint32_t count);

// Get timer ARR value
uint32_t timer_get_reload(Timer* timer);

// Sets the timer auto reload (value to count up to/down from). Period will be
// divided by the pre-scaler value.
// value: The value to set the auto reload register to
void timer_set_reload(Timer* timer, uint32_t reload);

// Set the callback function that is run when the timer triggers.
// callback: The function to call when the timer interrupt triggers
void timer_set_callback(Timer* timer, void (*callback)());

// Sets the timer pre-scaler (number of clock ticks before timer counts).
// value: The value to set the pre-scaler to
void timer_set_prescaler(Timer* timer, uint16_t value);

void timer_trigger_prescaler(Timer* timer);

#endif // TIMER_MOD_H
