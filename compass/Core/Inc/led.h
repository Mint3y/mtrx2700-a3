#ifndef LED_H
#define LED_H

#include "stm32f3xx_hal.h"
#include <stdint.h>

// Externally visible functions
uint8_t get_led_register(void);
void set_led_register(uint8_t state);

#endif // LED_H
