#include "beats.h"

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
static GPIO_TypeDef* INPUT_GPIO = GPIOA;
static uint16_t INPUT_PINS[4] = {
    INPUT_PIN_0,
    INPUT_PIN_1,
    INPUT_PIN_2,
	INPUT_PIN_3
};

// State
static Mode state;

// Display
static Beats display;
static bool display_showing;
static bool display_finalise_on_next;
static Timer display_off;

// Input
static Beats input;
static uint16_t INPUT_TIMEOUT_MS = 30000; // TODO configure timeout to something that isnt 30000ms
static uint16_t INPUT_POST_PUSH_THRESHOLD = 100;
// TODO (optional): input alignment factor (how close to the actual beat the player needs to hit)

// Levels
static uint32_t level_number = 0;
static uint32_t MAX_LEVEL = 3;
static int8_t pattern_test[] = {
	XOOO,
	XOOO
};
static int8_t pattern_1[] = {
    OXOO,
    XOOO,
    OOOX,
    OOOO,
    OOXO,
    XOOO
};
static int8_t* level_patterns[] = {
	pattern_1
};
static uint32_t level_on_periods[] = {
	1000,
	700,
	300
};
static uint32_t level_off_periods[] = {
	600,
	400,
	200
};

void init_beats() {
	// Enable clocks
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIOEEN;
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

	// Debug: Enable board LED output registers
	uint16_t *debug_output_reg = ((uint16_t *)&(GPIOE->MODER)) + 1;
	*debug_output_reg = 0x5555;

	// Setup display timers
    display.timer = timer_init(TIM2, 0x00, TIM2_IRQn, 1);
    timer_set_prescaler(&(display.timer), 8000 * 6 - 1);
    set_beats_frequency(&display, 1000);
    timer_set_callback(&(display.timer), 0x00);

    display_showing = false;
    display_finalise_on_next = false;

    display_off = timer_init(TIM4, display_all_off, TIM4_IRQn, 1);
    timer_set_prescaler(&display_off, 8000 * 6 - 1);
    timer_set_reload(&display_off, 800);
	timer_set_count(&display_off, 0);
	timer_set_upcount(&display_off);
	timer_enable(&display_off);

    // Setup input timer
    input.timer = timer_init(TIM3, 0x00, TIM3_IRQn, 1);
    timer_set_prescaler(&(input.timer), 8000 * 6 - 1);
    set_beats_frequency(&input, INPUT_TIMEOUT_MS);
    timer_set_callback(&(input.timer), 0x00);

    // Reset beat players
    reset_beats(&display);
    reset_beats(&input);
}

void clear_beats_leds() {
    HAL_GPIO_WritePin(LED_GPIO, LED_PINS_BITMASK, GPIO_PIN_RESET);

    // TODO remove debug
    uint8_t* led_register = ((uint8_t*)&(GPIOE->ODR)) + 1;
    *led_register = 0;
}

void reset_beats(Beats* beats) {
    // Stop beats timer
	timer_disable(&(beats->timer));
	timer_set_count(&(beats->timer), 0);
    timer_set_callback(&(beats->timer), 0x00);

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
	timer_set_reload(&(beats->timer), frequency_ms);
}

void start_beat_display() {
    timer_set_callback(&(display.timer), display_next);
	timer_restart(&(display.timer));
	timer_restart(&display_off);

	// TODO remove debug
    uint8_t* led_register = ((uint8_t*)&(GPIOE->ODR)) + 1;
    *led_register = 1;
}

void start_beat_input() {
    timer_set_callback(&(input.timer), input_pattern_next);
    timer_restart(&(input.timer));
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

    // Setup and start the beat display
    display.playing = true;
    display.index = 0;
    display.count = count;
    display.pattern = pattern;
    display.finally = finally;
    set_beats_frequency(&display, frequency_ms);
    start_beat_display();
}

// Callback to setup input challenge for the previously displayed beat pattern
void finally_input_displayed_pattern() {
	// Stop display
	timer_disable(&display_off);
	timer_set_count(&display_off, 0);

	display_showing = false;
	display_finalise_on_next = false;

	// Setup input
    input.playing = true;
    input.index = 0;
    input.count = display.count;
    input.pattern = display.pattern;
    set_beats_frequency(&input, INPUT_TIMEOUT_MS);

    // Start input timer
    start_beat_input();
}

void finally_challenge_success() {
	// Go to next level
    ++level_number;

    // Stop at final level
    if (level_number >= MAX_LEVEL) {
    	return;
    }

    // Setup and start the beat display
    display.playing = true;
    display.index = 0;
    display.count = count;
    display.pattern = pattern;
    display.finally = finally;
    set_beats_frequency(&display, frequency_ms);
    start_beat_display();

    // TODO: transmit level number
}

void finally_challenge_fail() {
    // TODO: transmit fail
}

void beats_test() {
//    HAL_GPIO_WritePin(LED_GPIO, LED_PIN_0, GPIO_PIN_SET);

	int16_t pat[] = {
		0b0000000010000000,
		0b0000000001000000,
		0b0000000000100000,
		0b0000000010000000
	};

	init_beats();
	display_pattern(pat, 4, 1000, finally_input_displayed_pattern);
}

// Timer interrupt reads code and compares input to pattern
// Reduce timer to 1 clock tick to only accept a single button press
// TODO: Button interrupts to light up LEDs (using input.code) while taking input
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    // Clear LED pins
    clear_beats_leds();

	// TODO Light up corresponding LED for a moment (hardware)

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
		return;
	}

	// Check if timer is past post push threshold
	uint32_t input_time_remaining = (timer_get_count(&(input.timer))
			                      -  timer_get_reload(&(input.timer)));
	if (input_time_remaining <= INPUT_POST_PUSH_THRESHOLD) {
		return;
	}

	// Timer is not past the threshold, set count really close to ARR
	timer_set_count(&(input.timer), INPUT_POST_PUSH_THRESHOLD);
}

// timer interrupt to read codes
void input_pattern_next() {
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

    // Go to the next beat code and reset the input
    ++input.index;
    input.code = 0;

    // Last code of beat pattern was played successfully
    if (input.index >= input.count) {
        input.finally = finally_challenge_success;
        finalise_beats(&input);
    }
}

void TIM3_IRQHandler() {
	// Clear interrupt flag
	TIM3->SR &= ~TIM_SR_UIF;

	if (input.timer.callback != 0x00) {
		input.timer.callback();
	}
}

void display_next() {
	// Final LED was previously displayed, finish
	if (display_finalise_on_next) {
		finalise_beats(&display);
		return;
	}

    // Stop beat display on errors in pattern or index
    if ((display.pattern == 0x00)
    ||  (display.index >= display.count)) {
//    	while (1) {}  // Debug
        finalise_beats(&display);
        return;
    }

    // Get the current beat code
    display.code = display.pattern[display.index];

    if (display_showing) {
        // Clear LEDs, set reload to level off time
        clear_beats_leds();
        timer_set_reload(&(display.timer), 200);
        display_showing = false;
    }
    else {
        // Set LED pins to LOW/HIGH as coded
        uint16_t pin_set = (uint16_t)display.code; // todo debug change this to 0 after
        for (int i = 0; i < LED_PIN_COUNT; ++i) {
            pin_set |= LED_PINS[i] * ((display.code >> (LED_PIN_COUNT - 1 - i)) && (1));
        }
        HAL_GPIO_WritePin(LED_GPIO, pin_set, GPIO_PIN_SET);
        // TODO change this (test)
        uint8_t* led_register = ((uint8_t*)&(GPIOE->ODR)) + 1;
        *led_register = display.code;

        // Go to the next beat code
        ++display.index;

        // Last code of beat pattern has been reached
        if (display.index >= display.count) {
        	display_finalise_on_next = true;
        }

        // Set reload to level on time
        timer_set_reload(&(display.timer), 1000);
        display_showing = true;
    }

    // Manual reset again
    timer_restart(&(display.timer));
}

void TIM2_IRQHandler() {
	// Clear interrupt flag
	if (TIM2->SR & TIM_SR_UIF) {
		TIM2->SR &= ~TIM_SR_UIF;
	}

	// Clear capture/compare flag
	if (TIM2->SR & TIM_SR_CC1IF) {
		TIM2->SR &= ~TIM_SR_CC1IF;
	}

	if (display.timer.callback != 0x00) {
		display.timer.callback();
	}
}

void display_all_off() {
    uint8_t* led_register = ((uint8_t*)&(GPIOE->ODR)) + 1;
    *led_register = 0;

    // Clear LED pins
    clear_beats_leds();
}

void TIM4_IRQHandler() {
	// Clear interrupt flag
	if (TIM4->SR & TIM_SR_UIF) {
		TIM4->SR &= ~TIM_SR_UIF;
	}

	// Clear capture/compare flag
	if (TIM4->SR & TIM_SR_CC1IF) {
		TIM4->SR &= ~TIM_SR_CC1IF;
	}

	if (display_off.callback != 0x00) {
		display_off.callback();
	}
}
