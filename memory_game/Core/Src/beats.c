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

// State
static Mode mode = UNINITIALISED;

// Debug skip
static uint32_t skip_presses = 0;
static const uint32_t SKIP_CODE = 57;

// Main timer
static Timer main_timer;
static void (*main_finally)() = 0x00;

// Display
static Beats display;
static bool display_finalise_on_next;
static bool input_finalised;
static bool user_is_inputting;

// Input
static Beats input;
static uint16_t INPUT_TIMEOUT_MS = 3000;
static uint16_t INPUT_RESET_SPEED = 400;

// Flashing component
static uint32_t flash_end_count = 0;
static uint32_t flashes_elapsed = 0;
static bool flash_on = true;

// Levels
static uint32_t level_number = 0;
static uint32_t MAX_LEVEL = 3;
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
	OXOO,
	OOXO
};
static const Level level_1 = {
	pattern_1,
	sizeof(pattern_1) / sizeof(pattern_1[0]),
	700,
	300,
	2000
};
static const Level level_2 = {
	pattern_2,
	sizeof(pattern_2) / sizeof(pattern_2[0]),
	400,
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

void set_beats_leds(uint16_t pins) {
	HAL_GPIO_WritePin(LED_GPIO, pins, GPIO_PIN_SET);
}

void clear_beats_leds() {
    HAL_GPIO_WritePin(LED_GPIO, LED_PINS_BITMASK, GPIO_PIN_RESET);
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
	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
	
	// Setup input from GPIO pins
	init_buttons();

	// Setup main timer
    main_timer = timer_init(TIM4, 0x00, TIM4_IRQn, 1);
    timer_set_prescaler(&main_timer, 8000 * 6 - 1);
    main_finally = 0x00;

    // Reset flags
    display_finalise_on_next = false;
    input_finalised = false;
    user_is_inputting = false;

    // Reset beat players
    reset_beats(&display);
    reset_beats(&input);
}

void reset_beats(Beats* beats) {
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

	// Level preparation delay and change state
	mode = FLASHING;
	main_finally = finally_start_active_level;
	flash_start(DEFAULT_FLASH_COUNT, DEFAULT_FLASH_PERIOD);
}

void finally_start_active_level() {
	// Validate active level
	if (active_level == 0x00) {
		return;
	}

	// Start active level
	display_pattern(active_level->pattern,
					active_level->pattern_length,
                    active_level->off_period,
                    finally_input_displayed_pattern);
}

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
    timer_set_reload(&main_timer, frequency_ms);

    // Start beat display
    mode = DISPLAY;
    timer_set_callback(&main_timer, display_next);
	timer_restart(&main_timer);
}

void finally_input_displayed_pattern() {
	// Change program state
    mode = INPUT;
	timer_disable(&main_timer);

	// Reset flags
	display_finalise_on_next = false;
	input_finalised = false;
	user_is_inputting = false;
	enable_debug_led();

	// Setup input
    input.playing = true;
    input.index = 0;
    input.count = display.count;
    input.pattern = display.pattern;

    // Start input
    timer_set_reload(&main_timer, INPUT_TIMEOUT_MS);
    timer_set_callback(&main_timer, input_pattern_next);
	timer_restart(&main_timer);
}

void module_complete_callback() {
	timer_disable(&main_timer);
	if (finally_module_complete != 0x00) {
		finally_module_complete();
	}
}

void finally_challenge_success() {
	// Change program state
	mode = DISPLAY;
	timer_disable(&main_timer);

	// Reset flags
	input_finalised = true;
	user_is_inputting = false;

	// Go to next level
    ++level_number;

    // Enable all LEDs upon completing the final level
    if (level_number >= MAX_LEVEL) {
    	// Enter module completion state
        mode = COMPLETE;
    	main_finally = finally_module_complete;
    	timer_set_callback(&main_timer, module_complete_callback);
    	timer_set_reload(&main_timer, module_completion_delay_ms);
    	timer_restart(&main_timer);

        // Light up all LEDs
    	enable_debug_led();
    	set_beats_leds(LED_PIN_0 | LED_PIN_1 | LED_PIN_2);
    	return;
    }

    // Play the next level
    play_level(level_number);
}

void finally_challenge_fail() {
	// Change program state
	mode = FAILED;

	// Clear beat LEDs
	input_finalised = true;
	user_is_inputting = false;
	clear_beats_leds();

	// Enter an infinite loop
	main_finally = finally_challenge_fail;
	flash_start(DEFAULT_FLASH_COUNT, DEFAULT_FLASH_PERIOD);
}

void blue_button_callback() {
	// Start the beats game
	switch (mode) {
	case UNINITIALISED:
		mode = FLASHING;
		init_beats();
		play_level(0);
		break;

	// Count presses while flashing
	case FLASHING:
		++skip_presses;
		break;

	default:
		break;
	}

	// Skip beats challenge
	if (skip_presses > 3 && skip_presses != SKIP_CODE) {
		skip_presses = SKIP_CODE;
		mode = COMPLETE;
		level_number = MAX_LEVEL;
		finally_challenge_success();
	}
}

void button_callback(uint16_t led_number) {
	// Check that the program is in input state
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

    // Light up pressed LED
    set_beats_leds(led_number);

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
	uint32_t input_time_remaining = (timer_get_count(&main_timer)
			                      -  timer_get_reload(&main_timer));
	if (input_time_remaining <= INPUT_RESET_SPEED) {
		return;
	}

	// Timer is not past the threshold, set count really close to ARR
	timer_set_count(&main_timer, INPUT_TIMEOUT_MS - INPUT_RESET_SPEED);
}

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

void display_next() {
	// Finished displaying after previous wait
	if (display_finalise_on_next) {
		finalise_beats(&display);
		clear_beats_leds();
		return;
	}

    // Stop beat display on errors in pattern or index
    if ((display.pattern == 0x00)
    ||  (display.index >= display.count)) {
        finalise_beats(&display);
        return;
    }

    // Get the current beat code
    display.code = display.pattern[display.index];

    // Extract beat code pins
    uint16_t pin_set = 0;
    for (int i = 0; i < LED_PIN_COUNT; ++i) {
    	if ((display.code >> i) & (1)) {
            pin_set |= LED_PINS[i];
    	}
    }

    // Set LEDs to HIGH/LOW as coded
    clear_beats_leds();
    set_beats_leds(pin_set);

    // Change timer callback to wait
    timer_set_callback(&main_timer, display_wait);
    timer_set_reload(&main_timer, active_level->on_period);
    timer_set_count(&main_timer, 1);
}

void display_wait() {
    clear_beats_leds();

    // Go to the next beat code
    ++display.index;

    // Last code of beat pattern has been reached
    if (display.index >= display.count) {
    	display_finalise_on_next = true;
    }

    // Change timer callback to next
    timer_set_callback(&main_timer, display_next);
    timer_set_reload(&main_timer, active_level->off_period);
    timer_set_count(&main_timer, 1);
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
}

void EXTI0_IRQHandler() {
	blue_button_callback();
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

