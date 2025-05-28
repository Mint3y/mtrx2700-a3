#include "led_game.h"
#include "timer.h"
#include <stdlib.h>

// These global variables track game state:
// - button_seed_time is set when the user presses the button (used to seed the RNG)
// - stop_chase is set to 1 when the button is pressed during the chase to stop it
// - target_led stores the randomly selected correct LED to stop on
volatile uint32_t button_seed_time = 0;
volatile uint8_t stop_chase = 0;
uint8_t target_led = 0;


void initialise_board(void) {
    // Enable clock for GPIOA
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

    // Clear mode bits for PA1–PA7 (each pin uses 2 bits in MODER)
    GPIOA->MODER &= ~(0x3FFF << 2);

    // Set PA1–PA7 as general-purpose outputs (0b01 for each pin)
    GPIOA->MODER |= (0x1555 << 2);

    // Turn off all LEDs initially
    GPIOA->ODR &= ~(0xFE);  // 0xFE = bits 1–7
}


void SetSingleLED(uint8_t led_index) {
    // Clear all LEDs first
    GPIOA->ODR &= ~(0xFE);

    // Only turn on LED if it's between 1 and 7
    if (led_index >= 1 && led_index <= 7) {
        GPIOA->ODR |= (1 << led_index);  // Set bit corresponding to LED
    }
}


void ShowTargetLED(void) {
    // Wait here until button_seed_time is set from the button ISR
    while (button_seed_time == 0);

    // Seed the random number generator
    srand(button_seed_time);

    // Choose a random LED from 1 to 7 (inclusive)
    target_led = (rand() % 7) + 1;

    // Flash the target LED briefly to show the user
    SetSingleLED(target_led);
    delay(500);
    SetSingleLED(0);  // Turn off all LEDs
    delay(200);
}


uint8_t PlayChaseGame(uint32_t delay_ms) {
    stop_chase = 0;         // Reset stop flag
    button_seed_time = 0;   // Reset button press for next game
    GPIOA->ODR &= ~(0xFE);  // Ensure all LEDs start off

    ShowTargetLED();        // Randomly flash target LED once

    uint8_t current_led = 1;  // Start from LED 1
    int8_t direction = 1;     // Move forward initially

    // Keep looping until the user presses the button
    while (!stop_chase) {
        SetSingleLED(current_led);  // Light up the current LED
        delay(delay_ms);            // Wait for the specified time

        // Reverse direction at the ends (LED 1 or 7)
        if (current_led == 7) direction = -1;
        else if (current_led == 1) direction = 1;

        // Move to the next LED
        current_led += direction;
    }

    // The final LED is the one that was lit just before the button was pressed
    uint8_t final_led = current_led - direction;

    // Check if the user stopped on the correct LED
    if (final_led == target_led) {
        TIM2->CCR1 = 0;  // Turn off PWM (e.g., motor or buzzer)
        return 1;        // User wins
    } else {
        // User guessed incorrectly → shake the motor 3 times to indicate failure
        for (int i = 0; i < 3; i++) {
            TIM2->CCR1 = 30000;  // Turn motor/buzzer on
            delay(150);
            TIM2->CCR1 = 0;      // Turn motor/buzzer off
            delay(150);
        }
        TIM2->CCR1 = 0;
        return 0;  // User loses
    }
}
