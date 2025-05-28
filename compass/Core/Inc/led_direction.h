#ifndef LED_DIRECTION_H
#define LED_DIRECTION_H

#include "stm32f3xx_hal.h"

// Define direction enum
typedef enum {
    DIR_N,
    DIR_NE,
    DIR_E,
    DIR_SE,
    DIR_S,
    DIR_SW,
    DIR_W,
    DIR_NW,
    DIR_NONE
} Direction;

// Function prototypes
Direction get_direction_from_heading(float heading);
void turn_on_direction_led(Direction dir);

#endif // LED_DIRECTION_H
