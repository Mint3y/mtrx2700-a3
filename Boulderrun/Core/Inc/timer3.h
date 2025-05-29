#include "stm32f303xc.h"
//This file was used in the group however the timer was switched to timer3 as to not impact the pwm.
volatile uint8_t delay_done = 0;

void TIM3_IRQHandler(void) {
    if (TIM3->SR & TIM_SR_UIF) {
        TIM3->SR &= ~TIM_SR_UIF;   // Clear interrupt flag
        delay_done = 1;            // Delay completed
    }
}

void delay(uint32_t ms) {
    delay_done = 0;

    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;  // Enable TIM3 clock
    (void)RCC->APB1ENR;                  // Read-back for delay

    TIM3->CR1 &= ~TIM_CR1_CEN;          // Stop timer
    TIM3->PSC = 47999;                  // 48 MHz / (47999+1) = 1 kHz = 1 ms tick
    TIM3->ARR = ms;                     // Auto-reload value
    TIM3->CNT = 0;
    TIM3->EGR = TIM_EGR_UG;             // Force update
    TIM3->SR &= ~TIM_SR_UIF;            // Clear update flag
    TIM3->DIER |= TIM_DIER_UIE;         // Enable update interrupt
    TIM3->CR1 |= TIM_CR1_OPM;           // One-pulse mode

    NVIC_SetPriority(TIM3_IRQn, 0);     // Highest priority
    NVIC_EnableIRQ(TIM3_IRQn);

    TIM3->CR1 |= TIM_CR1_CEN;           // Start the timer

    while (!delay_done);

    TIM3->DIER &= ~TIM_DIER_UIE;        // Disable interrupt
}
