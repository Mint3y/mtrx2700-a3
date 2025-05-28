#ifndef LED_GAME_H
#define LED_GAME_H

#include "stm32f3xx_hal.h"
#include <stdint.h>

// Functions
void initialise_board(void);
void SetSingleLED(uint8_t led_index);
void ShowTargetLED(void);
uint8_t PlayChaseGame(uint32_t delay_ms);

#endif // LED_GAME_H
