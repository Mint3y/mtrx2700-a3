#include "stm32f303xc.h"
#include <stdint.h>

#ifndef ADC_H
#define ADC_H
//The following code was taken from the MTRX2700 GitHub and altered to what was needed.
uint16_t ReadADC() {
	// request the process to start
	ADC1->CR |= ADC_CR_ADSTART;
	// Wait for the end of the first conversion
	while(!(ADC1->ISR & ADC_ISR_EOC));
	// read the first value
	uint16_t value = ADC1->DR;
	ADC1->ISR |= ADC_ISR_EOS;
	return value;
}

void enable_ADC(void){
	// enable the clock for ADC1
	RCC->AHBENR |= RCC_AHBENR_ADC12EN;

	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

	// set to synchronise the ADC with the clock
	ADC12_COMMON->CCR |= ADC12_CCR_CKMODE_0;

	// ADEN must be = 0 for configuration (is the default)
	ADC1->CR &= ~ADC_CR_ADVREGEN; // clear voltage regulator enable
	ADC1->CR |= ADC_CR_ADVREGEN_0; // set ADVREGEN TO 01
	ADC1->CR &= ~ADC_CR_ADCALDIF; // clear bit to enable Single-ended-input

	// calibrate the ADC (self calibration routine)
	ADC1->CR |= ADC_CR_ADCAL;
	while((ADC1->CR & ADC_CR_ADCAL) == ADC_CR_ADCAL); // Waiting for the calibration to finish

	// We want to read from two channels each sequence
	//  the first channel goes in SQ1
	//  the second channel goes in SQ2
	//  the number of channels to read = 1, so the L value is 0
	ADC1->SQR1 = 0;
	ADC1->SQR1 |= 0x01 << ADC_SQR1_SQ1_Pos; // set the request for channel 6
	ADC1->SQR1 |= 0x00 << ADC_SQR1_L_Pos; // set the number of channels to read

	// single shot mode
	ADC1->CFGR &= ~ADC_CFGR_CONT;

	// Enable the ADC
	ADC1->CR |= ADC_CR_ADEN;

	// Wait for the ADC to be ready.
	while (ADC1->ISR == 0);
}
#endif
