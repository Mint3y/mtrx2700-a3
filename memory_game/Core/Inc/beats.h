#ifndef BEATS_H
#define BEATS_H

#include <stdint.h>
#include <stdbool.h>

#include "stm32f3xx_hal.h"
#include "timer.h"

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

#define DEFAULT_FLASH_COUNT 3
#define DEFAULT_FLASH_PERIOD 1000

typedef struct {
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
	COMPLETE,
	DISPLAY,
	INPUT,
	DELAY,
	FLASHING
} Mode;

// The function to run upon completing the module
static void (*finally_module_complete)() = 0x00;
static uint32_t module_completion_delay_ms = 1000;

// Enables the debugging (yellow) LED
void enable_debug_led();

// Disables the debugging LED
void disable_debug_led();

// Sets LEDs according to the given GPIO pins
void set_beats_leds(uint16_t pins);

// Clears all LEDs connected to GPIO pins
void clear_beats_leds();

// Initialises the blue button and GPIO buttons
// Must be called to allow the blue button to access
// the program entry point
void init_buttons();

// Initialises the beats module
// Will be called after pressing the blue button if
// init buttons was previously called
void init_beats();

// Resets the contents of a beats struct
void reset_beats(Beats* beats);

// Performs a beat's finally function and then resets it
void finalise_beats(Beats* beats);

// Begins a sequence of flashes with the given parameters
// Will call the finally function of the main timer
void flash_start(uint32_t flash_count, uint32_t period);

// Callback used to perform the flashes
void flash_callback();

// Performs a flash sequence and sets the active level
// Changes main finally to start active level
void play_level(uint32_t level_number);

// Starts playing the active level
void finally_start_active_level();

// Display a given pattern
void display_pattern(int8_t*  pattern,
                     uint32_t count,
                     uint32_t frequency_ms,
                     void (*finally)());

// Callback to setup input challenge for the previously displayed beat pattern
void finally_input_displayed_pattern();

// Callback that executes finally module complete after the completion delay
void module_complete_callback();

// Callback representing input challenge success
// Plays the next level or turns all LEDs on if on the final level
void finally_challenge_success();

// Callback representing input challenge failure
// Enters an endless flashing loop
void finally_challenge_fail();

// Callback for the blue button
// Allows entry into the beats module
void blue_button_callback();

//  Callback for GPIO buttons
void button_callback(uint16_t led_number);

// Reads in the input from the buttons
void input_pattern_next();

// Displays the next pattern in the beatmap
void display_next();

// Displays the next pattern in the beatmap after a delay
void display_wait();

#endif // BEATS_H
