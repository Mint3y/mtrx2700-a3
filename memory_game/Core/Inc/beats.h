#ifndef BEATS_H
#define BEATS_H

#include <stdint.h>
#include <stdbool.h>

#include "stm32f3xx_hal.h"
#include "timer.h"

// Beats for rhythm game
// #define OOOO 0b00000000
// #define OOOX 0b00000001
// #define OOXO 0b00000010
// #define OOXX 0b00000011

// #define OXOO 0b00000100
// #define OXOX 0b00000101
// #define OXXO 0b00000110
// #define OXXX 0b00000111

// #define XOOO 0b00001000
// #define XOOX 0b00001001
// #define XOXO 0b00001010
// #define XOXX 0b00001011

// #define XXOO 0b00001100
// #define XXOX 0b00001101
// #define XXXO 0b00001110
// #define XXXX 0b00001111

// Beat definitions
#define OOOO 0b00000000
#define OOOX 0b00000001
#define OOXO 0b00000010
#define OXOO 0b00000100
#define XOOO 0b00001000

// LED pin definitions
#define LED_PIN_0 GPIO_PIN_0
#define LED_PIN_1 GPIO_PIN_1
#define LED_PIN_2 GPIO_PIN_2
#define LED_PIN_3 GPIO_PIN_3
#define LED_PIN_COUNT 4

#define LED_DEBUG_PIN GPIO_PIN_3

// Input button pin definitions
#define INPUT_PIN_0 GPIO_PIN_1
#define INPUT_PIN_1 GPIO_PIN_2
#define INPUT_PIN_2 GPIO_PIN_3
#define INPUT_PIN_3 GPIO_PIN_4

typedef struct {
	Timer    timer;
    bool     playing;
    uint32_t index;
    uint32_t count;
    int8_t   code;
    int8_t*  pattern;
    void (*finally)();
} Beats;

typedef struct {
	int8_t* pattern;
	uint32_t pattern_length;
	uint32_t on_period;
	uint32_t off_period;
	uint32_t input_timeout;
} Level;

typedef enum {
	UNINITIALISED,
	FAILED,
	IDLE,
	DISPLAY,
	INPUT,
	DELAY,
	FLASHING
} Mode;

void init_buttons();

// Initialises the beats module
void init_beats();

// Clears the LED output for the beats module
void clear_beats_leds();

// Resets a beat player (decommissions it)
void reset_beats(Beats* beats);

// Finalises a beat player
void finalise_beats(Beats* beats);

// Begins displaying beats
void start_beat_display();

void flash_start(uint32_t flash_count, uint32_t period);

void flash_callback();

// Setup and begin displaying a beat pattern
void display_pattern(int8_t*  pattern,
                     uint32_t count,
                     uint32_t frequency_ms,
                     void (*finally)());

void play_level(uint32_t level_number);

void finally_start_active_level();

// Callback to setup input challenge for the previously displayed beat pattern
void finally_input_displayed_pattern();

// Callback when beat challenge is passed
void finally_challenge_success();

// Callback when beat challenge is failed
void finally_challenge_fail();

// Debugging function
void beats_test();

void button_callback(uint16_t led_number);

void input_pattern_next();

void display_next();

#endif // BEATS_H
