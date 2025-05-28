#include "led.h"

static uint8_t led_register = 0;

uint8_t get_led_register(void) {
    return led_register;
}

void set_led_register(uint8_t state) {
    led_register = state;
    uint8_t *gpio_odr_upper_byte = ((uint8_t*)&(GPIOE->ODR)) + 1;
    *gpio_odr_upper_byte = state;
}
