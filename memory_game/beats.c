#include "beats.h"

static int8_t pattern_1[] = {
    OXOO,
    XOOO,
    OOOX,
    OOOO,
    OOXO,
    XOOO
};

// Beat player
static Beats display;
static Beats input;
static uint32_t level_number = 0;

// TODO: input settings
// input timeout frequency
// TODO: input alignment factor (how close to the actual beat the player needs to hit)

void reset_beats(Beats* beats) {
    // TODO: Stop beats timer

    // Reset the beats information
    beats->playing = false;
    beats->index = 0;
    beats->count = 0;
    beats->pattern = 0x00;
    beats->finally = 0x00;

    // TODO: override all LEDs with LOW
}

void init_beats() {
    reset_beats(&display);
    reset_beats(&input);

    // TODO: enable timer interrupt
    // TODO: set timer interrupt
}

void finalise_beats(Beats* beats) {
    if (beats->finally != 0x00) {
        beats->finally();
    }

    reset_beats(&beats);
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
void expect_displayed_pattern() {
    input.playing = true;
    input.index = 0;
    input.count = display.count;
    input.pattern = display.pattern;
    set_beats_frequency(&input, 1000); // TODO: replace with input timeout frequency

    // TODO: start input timer
}

void challenge_success() {
    ++level_number;
    // TODO: transmit level number
}

void challenge_fail() {
    // TODO: transmit fail
}

// TODO: for the following, reduce the timer to a really short period if it isn't shorter than that
// Timer interrupt reads code and compares input to pattern
// Reduce timer to 1 clock tick to only accept a single button press
// TODO: Button interrupts to light up LEDs (using input.code) while taking input
void led_pin_interrupt_to_rename_1() {
    input.code |= XOOO;
}

void led_pin_interrupt_to_rename_2() {
    input.code |= OXOO;
}

void led_pin_interrupt_to_rename_3() {
    input.code |= OOXO;
}

void led_pin_interrupt_to_rename_4() {
    input.code |= OOOX;
}

// timer interrupt to read codes
void timer_input_interrupt_to_rename() {
    // Stop accepting input on errors in pattern or index
    if ((input.pattern == 0x00)
    ||  (input.index >= input.count)) {
        stop_beat_player(&input);
        return;
    }

    // Input code did not match beat pattern
    if (input.code != input.pattern[input.index]) {
        input.finally = challenge_fail;
        finalise_beats(&input);
        return;
    }

    // Go to the next beat code
    ++input.index;

    // Last code of beat pattern was played successfully
    if (input.index >= input.count) {
        input.finally = challenge_success;
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

    // TODO: Set LED pins to LOW/HIGH as coded
    // TODO: This is temporary: Extract desired LED states from bitmask
    bool led_states[4] = {false, false, false, false};
    for (int i = 0; i < 4; --i) {
        led_states[i] = (display.code >> (3 - i)) && (1);
    }

    // Go to the next beat code
    ++display.index;

    // Last code of beat pattern has been reached
    if (display.index >= display.count) {
        finalise_beats(&display);
    }
}