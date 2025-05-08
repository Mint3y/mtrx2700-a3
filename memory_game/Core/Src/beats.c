#include "beats.h"

static int8_t pattern_1[] = {
    OXOO,
    XOOO,
    OOOX,
    OOOO,
    OOXO,
    XOOO
};

// The pins used for displaying LEDs
static GPIO_TypeDef* LED_GPIO = GPIOC;
static uint16_t LED_PINS[LED_PIN_COUNT] = {
    LED_PIN_0,
    LED_PIN_1,
    LED_PIN_2,
    LED_PIN_3
};
static uint16_t LED_PINS_BITMASK = (LED_PIN_0
                                 |  LED_PIN_1
                                 |  LED_PIN_2
                                 |  LED_PIN_3);

// The pins used for input
//static GPIO_TypeDef* INPUT_GPIO = GPIOA;
//static uint16_t INPUT_PINS[4] = {
//    INPUT_PIN_0,
//    INPUT_PIN_1,
//    INPUT_PIN_2,
//	INPUT_PIN_3
//};

// Beat player
static Beats display;
static Beats input;
static uint32_t level_number = 0;
// TODO: input settings
static uint16_t INPUT_TIMEOUT_MS = 30000; // TODO configure timeout to something that isnt 30000ms
// TODO: input alignment factor (how close to the actual beat the player needs to hit)


void init_beats() {
    reset_beats(&display);
    reset_beats(&input);

    // TODO: enable timer interrupt
    // TODO: set timer interrupt
}

void clear_beats_leds() {
    HAL_GPIO_WritePin(LED_GPIO, LED_PINS_BITMASK, GPIO_PIN_RESET);
}

void reset_beats(Beats* beats) {
    // TODO: Stop beats timer

    // Reset the beats information
    beats->playing = false;
    beats->index = 0;
    beats->count = 0;
    beats->pattern = 0x00;
    beats->finally = 0x00;

    // Clear LED pins
    clear_beats_leds();
}

void finalise_beats(Beats* beats) {
    if (beats->finally != 0x00) {
        beats->finally();
    }

    reset_beats(beats);
}

void set_beats_frequency(Beats* beats, uint32_t frequency_ms) {
    // TODO: set timer frequency
}

void start_beat_display() {
    // TODO: start display timer
}

// Setup to display a beat pattern
void display_pattern(int8_t*  pattern,
                     uint32_t count,
                     uint32_t frequency_ms,
                     void (*finally)()) {
    // Check that there isn't already a pattern being displayed
    if (display.playing == true) {
        return;
    }

    // Stop accepting input
    reset_beats(&input);

    // Setup the beat display
    display.playing = true;
    display.index = 0;
    display.count = count;
    display.pattern = pattern;
    display.finally = finally;
    set_beats_frequency(&display, frequency_ms);

    // Start beat display
    start_beat_display();
}

// Callback to setup input challenge for the previously displayed beat pattern
void finally_input_displayed_pattern() {
    input.playing = true;
    input.index = 0;
    input.count = display.count;
    input.pattern = display.pattern;
    set_beats_frequency(&input, INPUT_TIMEOUT_MS);

    // TODO: start input timer
}

void finally_challenge_success() {
    ++level_number;
    // TODO: transmit level number
}

void finally_challenge_fail() {
    // TODO: transmit fail
}

void beats_test() {
    HAL_GPIO_WritePin(LED_GPIO, LED_PIN_0, GPIO_PIN_SET);

//	init_beats();
//	display_pattern(pattern_1, 6, 1000, finally_input_displayed_pattern);
}


// TODO: for the following, reduce the timer to a really short period if it isn't shorter than that
// Timer interrupt reads code and compares input to pattern
// Reduce timer to 1 clock tick to only accept a single button press
// TODO: Button interrupts to light up LEDs (using input.code) while taking input
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    // Clear LED pins
    clear_beats_leds();

	// TODO Light up corresponding LED for a moment

	// Set input code depending on the button/s pressed
	switch (GPIO_Pin) {
	case INPUT_PIN_0:
		input.code |= XOOO;
		break;
	case INPUT_PIN_1:
		input.code |= OXOO;
		break;
	case INPUT_PIN_2:
		input.code |= OOXO;
		break;
	case INPUT_PIN_3:
		input.code |= OOOX;
		break;
	default:
		break;
	}

	// TODO Set timer really close to the ARR if it isn't past this value
}

// timer interrupt to read codes
void timer_input_interrupt_to_rename() {
    // Stop accepting input when pattern is invalid or index out of range
    if ((input.pattern == 0x00)
    ||  (input.index >= input.count)) {
        reset_beats(&input);
        return;
    }

    // Input code did not match beat pattern
    if (input.code != input.pattern[input.index]) {
        input.finally = finally_challenge_fail;
        finalise_beats(&input);
        return;
    }

    // Go to the next beat code
    ++input.index;

    // Last code of beat pattern was played successfully
    if (input.index >= input.count) {
        input.finally = finally_challenge_success;
        finalise_beats(&input);
    }
}

// timer interrupt to display pattern
void timer_display_interrupt_to_rename() {
    // Stop beat display on errors in pattern or index
    if ((display.pattern == 0x00)
    ||  (display.index >= display.count)) {
        finalise_beats(&display);
        return;
    }

    // Get the current beat code
    display.code = display.pattern[display.index];

    // Clear LED pins
    clear_beats_leds();

    // Set LED pins to LOW/HIGH as coded
    uint16_t pin_set = 0;
    for (int i = 0; i < LED_PIN_COUNT; --i) {
        pin_set |= LED_PINS[i] * ((display.code >> (LED_PIN_COUNT - 1 - i)) && (1));
    }
    HAL_GPIO_WritePin(LED_GPIO, pin_set, GPIO_PIN_SET);

    // Go to the next beat code
    ++display.index;

    // Last code of beat pattern has been reached
    if (display.index >= display.count) {
        finalise_beats(&display);
    }
}
