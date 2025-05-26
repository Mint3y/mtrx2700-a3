#include "timer.h"

Timer timer_init(TIM_TypeDef* timer_ptr,
				 void (*callback)(),
				 IRQn_Type timer_number,
				 int priority) {
	// Initialise timer struct
	Timer timer;
	timer.ptr = timer_ptr;
	timer.callback = callback;

	// Disable all interrupt requests while changing interrupt registers
	// Set priority and enable interrupt requests for the timer in the NVIC
	__disable_irq();
	NVIC_SetPriority(timer_number, priority);
	NVIC_EnableIRQ(timer_number);
	__enable_irq();

    // Enable capture/compare interrupt for timer
    timer.ptr->DIER |= TIM_DIER_CC1IE;

    return timer;
}

void timer_enable(Timer* timer) {
	timer->ptr->CR1 |= TIM_CR1_CEN;
}

void timer_disable(Timer* timer) {
	timer->ptr->CR1 &= ~TIM_CR1_CEN;
}

// Sets the timer counter back to 0 and enables the timer
void timer_restart(Timer* timer) {
	timer_disable(timer);
	uint32_t restart_value = 0;

	// Timer is down-counting
	if (timer->ptr->CR1 & TIM_CR1_DIR) {
		restart_value = timer_get_reload(timer);
	}

	timer_set_count(timer, restart_value);
	timer_enable(timer);
}

void timer_enable_oneshot(Timer* timer) {
	timer->ptr->CR1 |= TIM_CR1_OPM;
}

void timer_set_downcount(Timer* timer) {
	timer->ptr->CR1 |= TIM_CR1_DIR;
}

void timer_set_upcount(Timer* timer) {
	timer->ptr->CR1 &= ~TIM_CR1_DIR;
}

uint32_t timer_get_count(Timer* timer) {
	return timer->ptr->CNT;
}

void timer_set_count(Timer* timer, uint32_t count) {
	timer->ptr->CNT = count;
}

uint32_t timer_get_reload(Timer* timer) {
	return timer->ptr->ARR;
}

// Sets the timer auto reload (value to count up to/down from). Period will be
// divided by the pre-scaler value.
// value: The value to set the auto reload register to
void timer_set_reload(Timer* timer, uint32_t reload) {
	timer->ptr->ARR = reload;
}

// Set the callback function that is run when the timer triggers.
// callback: The function to call when the timer interrupt triggers
void timer_set_callback(Timer* timer, void (*callback)()) {
	timer->callback = callback;
}

// Sets the timer pre-scaler (number of clock ticks before timer counts).
// value: The value to set the pre-scaler to
void timer_set_prescaler(Timer* timer, uint16_t value) {
	timer->ptr->PSC = value;
	timer_trigger_prescaler(timer);
}

void timer_trigger_prescaler(Timer* timer) {
	// Store the previous reload value
	uint32_t reload = timer_get_reload(timer);

	// Make the reload value really small
	timer_set_reload(timer, 1);
	timer_set_count(timer, 0);

	// Short delay so timer can reset the prescaler upon reload
	for (int i = 0; i < 3; ++i) {
		asm("NOP");
	}

	// Reset the timer to the previous state
	timer_set_reload(timer, reload);
	timer_set_count(timer, 0);
}

void example_timer2_init() {
	Timer pulse_timer = timer_init(TIM2, 0x00, TIM2_IRQn, 1);
	timer_set_reload(&pulse_timer, 1000);
}

void example_callback_TIM2_IRQHandler() {
	// Clear timer interrupt flag
	if (TIM2->SR & TIM_SR_UIF) {
		// Your code here

		TIM2->SR &= ~TIM_SR_UIF;

		// Clear capture/compare interrupt flag
		if (TIM2->SR & TIM_SR_CC1IF) {
			TIM2->SR &= ~TIM_SR_CC1IF;
		}
	}
}


//// Enables the one-pulse timer option
//void enable_timer_onepulse() {
//	TIMER_STATE.timer->CR1 |= TIM_CR1_OPM;
//}
//
//// Disables the one-pulse timer option
//void disable_timer_onepulse() {
//	TIMER_STATE.timer->CR1 &= ~TIM_CR1_OPM;
//}
//
//void set_timer_oneshot_delay(uint32_t value) {
//	timer_set_reload(value);
//	TIMER_STATE.timer->CCR1 = value;
//}
//
//
//
//
//// Initialise timer and trigger an interrupt for a call back function per delay period
//
//void initialise_timer_a(int delay_ms, uint8_t *led_reg, PatternForCallback CallBack){
//
//	mode = 1;
//
////	pattern1 = CallBack;
//	set_timer_callback(CallBack);
//
//	Led_register = led_reg;
//
//	// Configure each timer cycle to 1ms
//	TIM2->PSC = 7999;
//	TIM2-> ARR = delay_ms;
//	TIM2 -> CNT = 0;
//
////	//Enable Interrupt
////    TIM2->DIER |= TIM_DIER_UIE;
////    NVIC_EnableIRQ(TIM2_IRQn);
//
//    //Enable Count
//    TIM2->CR1 |= TIM_CR1_CEN;
//
//
//}
//
//
//void initialise_timer_b(int delay_ms, PatternForNewPeriod CallBack){
//
//	mode = 2;
//
////	pattern2 = CallBack;
//	set_timer_callback(CallBack);
//
//	period = delay_ms;
//
//
//
//	TIM2->PSC = 7999;
//	TIM2-> ARR = period;
//	TIM2 -> CNT = 0;
//
//    TIM2->DIER |= TIM_DIER_UIE;
//    NVIC_EnableIRQ(TIM2_IRQn);
//    TIM2->CR1 |= TIM_CR1_CEN;
//
//
//}
//
//
//
//void oneshot_timer_start(uint32_t delay_ms, OneShotCallback callback, uint8_t *led_reg)
//{
////    oneshot_callback = callback;
//	set_timer_callback(callback);
//    Led_register = led_reg;
//
//
//    uint32_t ticks = delay_ms;
//
//
//    TIM2->PSC = 7999;
//
//    // Sets value for capture/compare event
//    TIM2->CCR1 = ticks;
//
//    // Set period of a single pulse
//    TIM2->ARR = ticks;
//
//    // Enable one-pulse mode
//    TIM2->CR1 |= TIM_CR1_OPM;
//
//    TIM2->DIER |= TIM_DIER_CC1IE;
//
//    TIM2->CNT = 0;
//
//    // Enable timer
//    TIM2->CR1 |= TIM_CR1_CEN;
//
//	NVIC_EnableIRQ(TIM2_IRQn);
//}
//
//
//
//// Interrupt service routine for call back function


//void TIM2_IRQHandler(void) {
//	// Update current state for interrupt
//	if(TIM2 -> SR & TIM_SR_UIF){
//
//		// Clear Flag for next update event
//	    TIM2->SR &= ~TIM_SR_UIF;
//
//	    TIMER_STATE.callback();
//	    return;
//
//
////	    if(mode == 1){
////			if(pattern1 != 0){
////			pattern1(Led_register);
////
////			}
////
////	    }
////
////	    else if (mode == 2){
////
////	    	if(pattern2 != 0){
////				pattern2(period);
////TIM_SR_CC1IF
////			}
////
////	    }
//	}
//
//	// oneshot first pulse
////	 if (TIM2->SR & TIM_SR_CC1IF)
////	    {
////	        // Disable capture/compare flag
////	        TIM2->SR &= ~TIM_SR_CC1IF;
////
////		    TIMER_STATE.callback();
////		    return;
////
////	        if (oneshot_callback != 0)
////	        {
////	            oneshot_callback(Led_register);
////
////	        }
////
////	        // Disable the Capture/Compare 1 interrupt
////	        TIM2->DIER &= ~TIM_DIER_CC1IE;
////
////
////	 }
//}
//
//void set_period(int delay_ms){
//    period = delay_ms;
//    TIM2->ARR = delay_ms;
//    TIM2->CNT = 0;
//}
//
//int get_period(){
//    return period;
//}
//
//// Call back function for changing LED patterns
//void change_pattern_a() {
////	static int count = 0;
////
////	// LED pattern array
////	uint8_t pattern_list[] = {0xFF, 0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80, 0x00};
////
////	*led_output_register = pattern_list[count];
////
////	// Count traverse through each index starting from 0 and loops back to start
////	count = (count + 1 ) % 9 ;
//}
//
//void change_pattern_b(void) {
////	*led_output_register ^= 0xFF;
//}
//
//void one_shot_pattern(void) {
////    *led_output_register = 0xFF;
//}


///**
// ******************************************************************************
// * @file           : main.c
// * @author         : Auto-generated by STM32CubeIDE
// * @brief          : Main program body
// ******************************************************************************
// * @attention
// *
// * Copyright (c) 2025 STMicroelectronics.
// * All rights reserved.
// *
// * This software is licensed under terms that can be found in the LICENSE file
// * in the root directory of this software component.
// * If no LICENSE file comes with this software, it is provided AS-IS.
// *
// ******************************************************************************
// */
//
//#include "stm32f303xc.h"
//#include <stdint.h>
//
//#define SYS_CLOCK_HZ 8000000
//
//typedef void (*callback_t)(void);
//
//static callback_t user_callback = 0;
//static uint32_t interval_ms = 0;  // Track current timer period
//static uint8_t oneshot_mode = 0;         // 0 = normal, 1 = oneshot
//static callback_t oneshot_callback = 0;  // Separate callback for one-shot
//
//
//// Enable clocks for GPIOE and TIM2
//void enable_clocks() {
//	RCC->AHBENR  |= RCC_AHBENR_GPIOEEN;
//
//	/*
//	LDR R0, =0x40021014       @ RCC->AHBENR
//	LDR R1, [R0]
//	ORR R1, R1, #(1 << 21)    @ GPIOEEN
//	STR R1, [R0]
//	*/
//
//	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
//
//	/*
//	LDR R0, =0x4002101C       @ RCC->APB1ENR
//	LDR R1, [R0]
//	ORR R1, R1, #(1 << 0)     @ TIM2EN
//	STR R1, [R0]
//	*/
//
//	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
//}
//
//// Configure PE8–PE15 as output
//void initialise_board() {
//	uint16_t *led_output_registers = ((uint16_t *)&(GPIOE->MODER)) + 1;
//	*led_output_registers = 0x5555;
//
//	/*
//	@ Set PE8–15 to output mode (MODER[17:16] = 01 for each)
//	LDR R0, =0x48011000       @ GPIOE base
//	ADD R0, R0, #0x04         @ MODER offset
//	LDR R1, =0x5555
//	STR R1, [R0, #0x04]       @ Write to upper half of MODER
//	*/
//}
//
//// Force prescaler reload
//void trigger_prescaler() {
//	TIM2->ARR = 0x01;
//	TIM2->CNT = 0x00;
//	asm("NOP"); asm("NOP"); asm("NOP");
//	TIM2->ARR = 0xFFFFFFFF;
//
//	/*
//	@ Force update event so that PSC is loaded
//	LDR R0, =0x4000002C       @ TIM2->ARR
//	MOV R1, #1
//	STR R1, [R0]
//
//	LDR R0, =0x40000024       @ TIM2->CNT
//	MOV R1, #0
//	STR R1, [R0]
//
//	NOP
//	NOP
//	NOP
//
//	LDR R0, =0x4000002C
//	LDR R1, =0xFFFFFFFF
//	STR R1, [R0]
//	*/
//}
//// Initialize TIM2 to generate periodic interrupts
//void timer_init(uint32_t interval, callback_t cb) {
//	user_callback = cb;
//	interval_ms = interval;
//
//	uint32_t prescaler = 7999;           // 8MHz / (7999+1) = 1kHz
//	TIM2->PSC = prescaler;               // Set prescaler
//	trigger_prescaler();                 // Load PSC value
//
//	TIM2->ARR = interval_ms;             // Auto-reload value
//	TIM2->DIER |= TIM_DIER_UIE;          // Enable update interrupt
//	TIM2->CR1  |= TIM_CR1_CEN;           // Start the timer
//
//	/*
//	@ Set PSC
//	LDR R0, =0x40000028       @ TIM2->PSC
//	LDR R1, =7999
//	STR R1, [R0]
//
//	@ Set ARR
//	LDR R0, =0x4000002C
//	LDR R1, =<interval_ms>
//	STR R1, [R0]
//
//	@ Enable interrupt
//	LDR R0, =0x4000000C       @ TIM2->DIER
//	LDR R1, [R0]
//	ORR R1, R1, #1
//	STR R1, [R0]
//
//	@ Start timer
//	LDR R0, =0x40000000       @ TIM2->CR1
//	LDR R1, [R0]
//	ORR R1, R1, #1
//	STR R1, [R0]
//	*/
//
//	NVIC_EnableIRQ(TIM2_IRQn);           // NVIC global IRQ enable
//}
//// Call callback if UIF (periodic)
//void TIM2_IRQHandler(void) {
//	if (TIM2->SR & TIM_SR_UIF) {
//		TIM2->SR &= ~TIM_SR_UIF;  // Clear update flag
//
//		if (user_callback) {
//			user_callback();  // Periodic callback
//		}
//	}
//}
//// Function to blink one led
//void blink_led1(void) {
//    uint8_t *led_output_register = ((uint8_t*)&(GPIOE->ODR)) + 1;
//    static uint8_t state = 0;
//    const uint8_t mask = 0b00000001;  // Blink PE8, PE10, PE12, PE14
//
//    state ^= mask;
//    *led_output_register = (*led_output_register & ~mask) | (state & mask);
//}
//// Change TIM2 period
//void reset_period(uint32_t period) {
//	interval_ms = period;
//	TIM2->ARR = interval_ms;
//	TIM2->CNT = 0;
//	TIM2->EGR |= TIM_EGR_UG;
//
//	/*
//	@ Change ARR and force update
//	LDR R0, =0x4000002C       @ TIM2->ARR
//	LDR R1, =<period>
//	STR R1, [R0]
//
//	LDR R0, =0x40000024       @ TIM2->CNT
//	MOV R1, #0
//	STR R1, [R0]
//
//	LDR R0, =0x40000014       @ TIM2->EGR
//	MOV R1, #1
//	STR R1, [R0]
//	*/
//}
//
//// Return period
//uint32_t get_period(void) {
//	return interval_ms;
//}
//
//// Call callback if UIF (oneshot)
//void TIM3_IRQHandler(void) {
//    if (TIM3->SR & TIM_SR_UIF) {
//        TIM3->SR &= ~TIM_SR_UIF;  // Clear update flag
//
//        if (oneshot_mode && oneshot_callback) {
//            callback_t cb = oneshot_callback;
//            oneshot_callback = 0;
//            oneshot_mode = 0;
//            cb();  // One-shot callback
//        }
//    }
//}
//
//
//
//void start_oneshot_timer_TIM3(uint32_t delay_ms, callback_t cb) {
//	TIM3->SR &= ~TIM_SR_UIF;   // <<< Clear any pending update flag
//    oneshot_mode = 1;
//    oneshot_callback = cb;
//
//    TIM3->CR1 = 0;         // Disable timer
//    TIM3->CNT = 0;
//
//    TIM3->PSC = 7999;      // 1ms tick (8MHz / (7999 + 1) = 1kHz)
//    TIM3->ARR = delay_ms;
//
//    TIM3->EGR |= TIM_EGR_UG;  // <<< FORCE UPDATE >>> (loads PSC & ARR)
//    TIM3->SR &= ~TIM_SR_UIF;   // <<< Clear any pending update flag
//
//    TIM3->DIER |= TIM_DIER_UIE;   // Enable interrupt
//    TIM3->CR1 |= TIM_CR1_OPM;     // One-pulse mode
//    TIM3->CR1 |= TIM_CR1_CEN;     // Start timer
//
//    NVIC_EnableIRQ(TIM3_IRQn);    // Enable TIM3 interrupt
//}
//
//
//
//// Blink LED on PE15
//void blink_led2(void) {
//    uint8_t *led_output_register = ((uint8_t*)&(GPIOE->ODR)) + 1;
//    static uint8_t state = 0;
//    const uint8_t mask = 0b10000000;
//
//    state ^= mask;  // Toggle only bits in the mask
//    *led_output_register = (*led_output_register & ~mask) | (state & mask);
//
//
//	/*
//	@ Toggle upper byte of ODR (PE8–15)
//	LDR R0, =0x48011014       @ GPIOE->ODR
//	LDRB R1, [R0, #1]
//	EOR R1, R1, #0xFF
//	STRB R1, [R0, #1]
//	*/
//}
//
//
//volatile uint32_t current_period = 0;
//// === Main program ===
//int main(void) {
//	enable_clocks();                     // Enable GPIOE and TIM2 clocks
//	initialise_board();                  // Set PE8–15 as output
//	timer_init(1000, blink_led1);        // Call blink_leds every 1000ms
//	current_period = get_period();
//	for (volatile int i = 0; i < 8000000; ++i);  // Simple delay
//	blink_led2();
//	start_oneshot_timer_TIM3(2000, blink_led2);  // Call blink_led2 after 2 seconds
//
//	for (volatile int i = 0; i < 8000000; ++i);  // Simple delay
//	reset_period(5000);                  // Change period to 5s
//	current_period = get_period();
//
//	while (1) {
//
//	}
//}
