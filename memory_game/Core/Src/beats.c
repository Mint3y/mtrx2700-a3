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
                                 |  LED_PIN_2);

// The pins used for input
static GPIO_TypeDef* INPUT_GPIO = GPIOA;
static uint16_t INPUT_PINS[4] = {
    INPUT_PIN_0,
    INPUT_PIN_1,
    INPUT_PIN_2,
	INPUT_PIN_3
};

// State
static Mode mode = UNINITIALISED;

// Display
static Beats display;
static bool display_showing;
static bool display_finalise_on_next;
static bool input_finalised;
static bool user_is_inputting;
static Timer display_off;

// Input
static Beats input;
static uint16_t INPUT_TIMEOUT_MS = 3000; // TODO configure timeout to something that isnt 30000ms
static uint16_t INPUT_RESET_SPEED = 400;
// TODO (optional): input alignment factor (how close to the actual beat the player needs to hit)

// Flashing mode
static Timer flasher;

static Timer main_timer;
static void (*main_finally)() = 0x00;

// Levels
static uint32_t level_number = 0;
static uint32_t MAX_LEVEL = 3;
static int8_t pattern_test[] = {
	XOOO,
	OXOO,
	OXOO,
	OOXO,
	OOXO,
	OOOX
};
static int8_t pattern_1[] = {
    OOXO,
    OXOO,
    OOOX,
    OOOX,
    OOXO,
    OXOO
};
static int8_t pattern_2[] = {
	OXOO,
	OXOO,
	OOXO,
	OXOO,
	OOOX
};
static int8_t pattern_3[] = {
	OOOX,
	OOOX,
	OOOX,
	OXOO,
    OOXO,
    OXOO,
    OOOX,
	OXOO
};
static const Level level_1 = {
	pattern_1,
	sizeof(pattern_1) / sizeof(pattern_1[0]),
	1000,
	600,
	2000
};
static const Level level_2 = {
	pattern_2,
	sizeof(pattern_2) / sizeof(pattern_2[0]),
	600,
	400,
	2000
};
static const Level level_3 = {
	pattern_3,
	sizeof(pattern_3) / sizeof(pattern_3[0]),
	400,
	200,
	2000
};
static Level levels[] = {
	level_1,
	level_2,
	level_3
};
static Level* active_level = 0x00;

void enable_debug_led() {
	HAL_GPIO_WritePin(LED_GPIO, LED_DEBUG_PIN, GPIO_PIN_SET);
}

void disable_debug_led() {
	HAL_GPIO_WritePin(LED_GPIO, LED_DEBUG_PIN, GPIO_PIN_RESET);
}

void clear_beats_leds() {
    HAL_GPIO_WritePin(LED_GPIO, LED_PINS_BITMASK, GPIO_PIN_RESET);
}

void beats_test() {
	init_beats();
	display_pattern(pattern_1, 6, 1000, finally_input_displayed_pattern);
}

void init_buttons() {
	// Disable the interrupts while messing around with the settings
	//  otherwise can lead to strange behaviour
	__disable_irq();

	// Enable the system configuration controller (SYSCFG in RCC)
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

	// External Interrupts details on large manual page 294)
	// PA0 is on interrupt EXTI0 large manual - page 250
	// EXTI0 in  SYSCFG_EXTICR1 needs to be 0x00 (SYSCFG_EXTICR1_EXTI0_PA)
	SYSCFG->EXTICR[0] = SYSCFG_EXTICR1_EXTI0_PA;
	SYSCFG->EXTICR[0] = SYSCFG_EXTICR1_EXTI1_PA;
	SYSCFG->EXTICR[0] = SYSCFG_EXTICR1_EXTI2_PA;
	SYSCFG->EXTICR[0] = SYSCFG_EXTICR1_EXTI3_PA;
//	SYSCFG->EXTICR[1] = SYSCFG_EXTICR2_EXTI4_PA;

	//  Select EXTI0 interrupt on falling edge
	EXTI->RTSR |= EXTI_RTSR_TR0; // rising edge of EXTI line 0 (includes PA0)
	EXTI->FTSR |= EXTI_FTSR_TR1; // falling edge of EXTI line 0 (includes PA0)
	EXTI->FTSR |= EXTI_FTSR_TR2; // falling edge of EXTI line 0 (includes PA0)
	EXTI->FTSR |= EXTI_FTSR_TR3; // falling edge of EXTI line 0 (includes PA0)

	// Disable rising edge triggers
	EXTI->RTSR &= ~EXTI_RTSR_TR1; // rising edge of EXTI line 0 (includes PA0)
	EXTI->RTSR &= ~EXTI_RTSR_TR2; // rising edge of EXTI line 0 (includes PA0)
	EXTI->RTSR &= ~EXTI_RTSR_TR3; // rising edge of EXTI line 0 (includes PA0)

//	EXTI->FTSR |= EXTI_FTSR_TR4; // falling edge of EXTI line 0 (includes PA0)

	// set the interrupt from EXTI line 0 as 'not masked' - as in, enable it.
	EXTI->IMR |= EXTI_IMR_MR0;
	EXTI->IMR |= EXTI_IMR_MR1;
	EXTI->IMR |= EXTI_IMR_MR2;
	EXTI->IMR |= EXTI_IMR_MR3;
//	EXTI->IMR |= EXTI_IMR_MR4;

	// Tell the NVIC module that EXTI0 interrupts should be handled
	NVIC_SetPriority(EXTI0_IRQn, 1);
	NVIC_EnableIRQ(EXTI0_IRQn);
	NVIC_SetPriority(EXTI1_IRQn, 1);
	NVIC_EnableIRQ(EXTI1_IRQn);
	NVIC_SetPriority(EXTI2_TSC_IRQn, 1);
	NVIC_EnableIRQ(EXTI2_TSC_IRQn);
	NVIC_SetPriority(EXTI3_IRQn, 1);
	NVIC_EnableIRQ(EXTI3_IRQn);
//	NVIC_SetPriority(EXTI4_IRQn, 1);
//	NVIC_EnableIRQ(EXTI4_IRQn);

	// Re-enable all interrupts (now that we are finished)
	__enable_irq();
}

void init_beats() {
	// Enable clocks
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIOEEN;
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN | RCC_APB1ENR_TIM3EN | RCC_APB1ENR_TIM4EN;
	
	// Setup input from GPIO pins
	init_buttons();

	// Debug: Enable board LED output registers
	// uint16_t *debug_output_reg = ((uint16_t *)&(GPIOE->MODER)) + 1;
	// *debug_output_reg = 0x5555;

	// Setup timers and beat displayers
    display.timer = timer_init(TIM2, 0x00, TIM2_IRQn, 1);
    timer_set_prescaler(&(display.timer), 8000 * 6 - 1);

    display_off = timer_init(TIM4, 0x00, TIM4_IRQn, 1);
    timer_set_prescaler(&display_off, 8000 * 6 - 1);

    input.timer = timer_init(TIM3, 0x00, TIM3_IRQn, 1);
    timer_set_prescaler(&(input.timer), 8000 * 6 - 1);



    
    main_timer = timer_init(TIM4, 0x00, TIM4_IRQn, 1);
    timer_set_prescaler(&main_timer, 8000 * 6 - 1);
    main_finally = 0x00;



    // Reset flags
    display_showing = false;
    display_finalise_on_next = false;
    input_finalised = false;
    user_is_inputting = false;

    // Reset beat players
    reset_beats(&display);
    reset_beats(&input);
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
    beats->code = 0;
    beats->pattern = 0x00;
    beats->finally = 0x00;
}

void finalise_beats(Beats* beats) {
    if (beats->finally != 0x00) {
        beats->finally();
    }

    reset_beats(beats);
}

static uint32_t flash_end_count = 0;
static uint32_t flashes_elapsed = 0;
static bool flash_on = true;

void flash_start(uint32_t flash_count, uint32_t period) {
	// Store flash parameters
	flash_end_count = 3;
	flashes_elapsed = 0;
	flash_on = true;

	// Setup flash timer
	timer_disable(&main_timer);
	timer_set_callback(&main_timer, flash_callback);
	timer_set_reload(&main_timer, period / 2);
	timer_restart(&main_timer);
}

void flash_callback() {
	// Show debug LED
	if (flash_on == true) {
		flash_on = false;
		enable_debug_led();
	}
	
	// Flashes increment after off period, hide debug LED
	else {
		flash_on = true;
		disable_debug_led();
		++flashes_elapsed;
	}

	// Flash count reached
	if (flashes_elapsed >= flash_end_count) {
		main_finally();
	}
}

void play_level(uint32_t level_number) {
	// Validate level number
	if (level_number >= MAX_LEVEL) {
		return;
	}

	// Set the level
	active_level = levels + level_number;

	// Level preparation delay
	main_finally = finally_start_active_level;
	flash_start(3, 1000);
}

void finally_start_active_level() {
	// Validate active level
	if (active_level == 0x00) {
		return;
	}

	// Start active level
	display_pattern(active_level->pattern,
					active_level->pattern_length,
                    active_level->on_period,
                    finally_input_displayed_pattern);
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
    timer_set_reload(&(display.timer), frequency_ms);

    // Start beat display
    mode = DISPLAY;
    timer_set_callback(&(display.timer), display_next);
	timer_restart(&(display.timer));
	timer_restart(&display_off);
}

// Callback to setup input challenge for the previously displayed beat pattern
void finally_input_displayed_pattern() {
	// Stop display
	timer_disable(&(display.timer));
	timer_disable(&display_off);
	timer_set_count(&display_off, 0);
	timer_set_callback(&display_off, 0x00);

	display_showing = false;
	display_finalise_on_next = false;
	input_finalised = false;
	user_is_inputting = false;
	enable_debug_led();

	// Setup input
    input.playing = true;
    input.index = 0;
    input.count = display.count;
    input.pattern = display.pattern;
    timer_set_reload(&(input.timer), INPUT_TIMEOUT_MS);

    // Start input
    mode = INPUT;
    timer_enable(&(input.timer));
    timer_set_count(&(input.timer), 1);
    timer_set_callback(&(input.timer), input_pattern_next);
}

void finally_challenge_success() {
    mode = DISPLAY;
	input_finalised = true;
	user_is_inputting = false;
	enable_debug_led();
    HAL_GPIO_WritePin(LED_GPIO, LED_PIN_0 | LED_PIN_1 | LED_PIN_2, GPIO_PIN_SET);

	// Go to next level
    ++level_number;

    // Stop at final level
    if (level_number >= MAX_LEVEL) {
        mode = IDLE;
    	return;
    }

//    play_level(level_number);



    // TODO Setup and start the beat display
//    display.playing = true;
//    display.index = 0;
//    display.count = count;
//    display.pattern = pattern;
//    display.finally = finally;
//    set_beats_frequency(&display, frequency_ms);
//    start_beat_display();

    // TODO: transmit level number
}

void finally_challenge_fail() {
	mode = FAILED;

	input_finalised = true;
	user_is_inputting = false;
	enable_debug_led();
	clear_beats_leds();

    // TODO: transmit fail
}

void EXTI0_IRQHandler() {
	// Start the beats game
	if (mode == UNINITIALISED || FAILED) {
		mode = IDLE;
		init_beats();
		play_level(0);
	}

	EXTI->PR |= EXTI_PR_PR0;
}

void EXTI1_IRQHandler() {
	button_callback(LED_PIN_0);
	EXTI->PR |= EXTI_PR_PR1;
}

void EXTI2_TSC_IRQHandler() {
	button_callback(LED_PIN_1);
	EXTI->PR |= EXTI_PR_PR2;
}

void EXTI3_IRQHandler() {
	button_callback(LED_PIN_2);
	EXTI->PR |= EXTI_PR_PR3;
}

// Timer interrupt reads code and compares input to pattern
// Reduce timer to 1 clock tick to only accept a single button press
void button_callback(uint16_t led_number) {
	// Check that we're in input mode
	if (mode != INPUT) {
		return;
	}

	// Ignore incoming if something has already been pressed but not processed
	if (input.code != 0) {
		return;
	}

	// Enable input registration
	user_is_inputting = true;
	disable_debug_led();

    // Clear LED pins
    clear_beats_leds();
    HAL_GPIO_WritePin(LED_GPIO, led_number, GPIO_PIN_SET);

	// Set input code depending on the button/s pressed
	switch (led_number) {
	case LED_PIN_0:
		input.code |= OOOX;
		break;
	case LED_PIN_1:
		input.code |= OOXO;
		break;
	case LED_PIN_2:
		input.code |= OXOO;
		break;
	default:
		return;
	}

	// Check if timer is past post push threshold
	uint32_t input_time_remaining = (timer_get_count(&(input.timer))
			                      -  timer_get_reload(&(input.timer)));
	if (input_time_remaining <= INPUT_RESET_SPEED) {
		return;
	}

	// Timer is not past the threshold, set count really close to ARR
	timer_set_count(&(input.timer), INPUT_TIMEOUT_MS - INPUT_RESET_SPEED);
}

// timer interrupt to read codes
void input_pattern_next() {
	clear_beats_leds();

	// User must perform first input
	if (user_is_inputting == false) {
		return;
	}

	// Stop when input is complete
	if (input_finalised) {
		return;
	}

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
	if (TIM3->SR & TIM_SR_UIF) {
		TIM3->SR &= ~TIM_SR_UIF;

		// Clear capture/compare flag
		if (TIM3->SR & TIM_SR_CC1IF) {
			TIM3->SR &= ~TIM_SR_CC1IF;
		}
	}

	if (input.timer.callback != 0x00) {
		input.timer.callback();
	}
}

void display_next() {
	// Final LED was previously displayed, finish
	if (display_finalise_on_next) {
		finalise_beats(&display);
		clear_beats_leds();
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

    // Set LED pins to LOW/HIGH as coded
    uint16_t pin_set = 0;
    for (int i = 0; i < LED_PIN_COUNT; ++i) {
    	if ((display.code >> i) & (1)) {
            pin_set |= LED_PINS[i];
    	}
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

//    if (display_showing) {
//        // Clear LEDs, set reload to level off time
//        clear_beats_leds();
//        timer_set_reload(&(display.timer), 200);
//        display_showing = false;
//    }
//    else {
//
//        // Set reload to level on time
//        timer_set_reload(&(display.timer), 1000);
//        display_showing = true;
//    }

    // Reset display off timer
//    timer_restart(&(display.timer));
//	timer_restart(&display_off);
	timer_set_count(&display_off, 1);
}

void TIM2_IRQHandler() {
	// Clear interrupt flag
	if (TIM2->SR & TIM_SR_UIF) {
		TIM2->SR &= ~TIM_SR_UIF;

		// Clear capture/compare flag
		if (TIM2->SR & TIM_SR_CC1IF) {
			TIM2->SR &= ~TIM_SR_CC1IF;
		}
	}

	if (display.timer.callback != 0x00) {
		display.timer.callback();
	}
}

void TIM4_IRQHandler() {
	// Clear interrupt flag
	if (TIM4->SR & TIM_SR_UIF) {
		TIM4->SR &= ~TIM_SR_UIF;

		// Clear capture/compare flag
		if (TIM4->SR & TIM_SR_CC1IF) {
			TIM4->SR &= ~TIM_SR_CC1IF;
		}
	}

	// Run callback
	if (main_timer.callback != 0x00) {
		main_timer.callback();
	}

//	if (display_off.callback != 0x00) {
//		display_off.callback();
//	}
}
