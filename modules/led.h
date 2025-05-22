#include "stm32f303xc.h"
#include <stdint.h>

#ifndef LED_H
#define LED_H
//Contains the base register for the LEDs for the functions to use
static uint8_t led_register = 0;

//Return the led_register when the function is called
uint8_t get_led_register(void){
	return led_register;
}
//Loads in the base register of the GPIOE and sets to the new given state to display on the LEDs
void set_led_register(uint8_t state){
	led_register = state;
	uint8_t *led_register = ((uint8_t*)&(GPIOE->ODR)) + 1;
	*led_register = state;
}
#endif
