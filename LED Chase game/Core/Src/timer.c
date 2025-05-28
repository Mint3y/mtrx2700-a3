#include "stm32f303xc.h"
#include "timer.h"

volatile uint8_t delay_done = 0;

void TIM2_IRQHandler(void) {
    if (TIM2->SR & TIM_SR_UIF) {
        TIM2->SR &= ~TIM_SR_UIF;   // Clear interrupt flag
        delay_done = 1;            // Delay completed
    }
}

void delay(uint32_t ms) {
    delay_done = 0;

    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    (void)RCC->APB1ENR;

    TIM2->CR1 &= ~TIM_CR1_CEN;     // Stop timer if running
    TIM2->PSC = 47999;              // Assuming 8 MHz clock: 1ms tick
    TIM2->ARR = ms;
    TIM2->CNT = 0;
    TIM2->EGR = TIM_EGR_UG;
    TIM2->SR &= ~TIM_SR_UIF;       // Clear any pending flags
    TIM2->DIER |= TIM_DIER_UIE;
    TIM2->CR1 |= TIM_CR1_OPM;      // One-pulse mode

    NVIC_SetPriority(TIM2_IRQn, 0); // Highest priority (0 is highest)
    NVIC_EnableIRQ(TIM2_IRQn);

    TIM2->CR1 |= TIM_CR1_CEN;

    while (!delay_done);

    TIM2->DIER &= ~TIM_DIER_UIE; // Disable interrupt after delay
}
