#include <stdint.h>
#include "stm32f303xc.h"
#include <string.h>
#include <ctype.h>
#include "timer.h"

// Morse timing constants in milliseconds
// These define how long each dot, dash, and spacing delay will be
#define DOT_DURATION     200    // Duration of a dot signal
#define DASH_DURATION    1600   // Duration of a dash signal (typically 3x dot, but this is exaggerated for clarity)
#define SYMBOL_SPACE     300    // Delay between symbols (dot/dash) within the same letter
#define LETTER_SPACE     2000   // Delay between letters
#define WORD_SPACE       1400   // Delay between words

// Manually activate PWM output by setting TIM2 compare register high
// This effectively turns on the output signal (e.g., LED or buzzer)
static void pwm_on() {
    TIM2->CCR1 = 30000;  // Set PWM duty cycle (adjust as needed for your hardware)
}

// Deactivate PWM output by setting compare register to 0
static void pwm_off() {
    TIM2->CCR1 = 0;  // Turns off the output signal
}

// Morse code lookup table: maps each alphabet character to its Morse representation
typedef struct {
    char c;             // The character (A-Z)
    const char *code;   // Corresponding Morse code string
} MorseCode;

// Table of uppercase English letters and their Morse codes
static const MorseCode morse_table[] = {
    {'A', ".-"},   {'B', "-..."}, {'C', "-.-."}, {'D', "-.."},  {'E', "."},
    {'F', "..-."}, {'G', "--."},  {'H', "...."}, {'I', ".."},   {'J', ".---"},
    {'K', "-.-"},  {'L', ".-.."}, {'M', "--"},   {'N', "-."},   {'O', "---"},
    {'P', ".--."}, {'Q', "--.-"}, {'R', ".-."},  {'S', "..."},  {'T', "-"},
    {'U', "..-"},  {'V', "...-"}, {'W', ".--"},  {'X', "-..-"}, {'Y', "-.--"},
    {'Z', "--.."}
};

// This function finds the Morse code string for a given character
// Input character is converted to uppercase before matching
static const char* lookup_morse(char c) {
    c = toupper((unsigned char)c);  // Convert to uppercase for case-insensitive matching
    for (int i = 0; i < sizeof(morse_table)/sizeof(MorseCode); i++) {
        if (morse_table[i].c == c)  // If match found in table
            return morse_table[i].code;
    }
    return NULL;  // Return NULL if character not found (e.g., punctuation)
}

// This function plays a Morse message by blinking or sounding each character
void play_morse(const char *message) {
    // Loop through each character of the input message string
    for (int i = 0; message[i] != '\0'; i++) {
        // If the character is a space, it's a word gap
        if (message[i] == ' ') {
            delay(WORD_SPACE);  // Wait between words
            continue;           // Skip to next character
        }

        // Look up the Morse code for the current letter
        const char *morse = lookup_morse(message[i]);
        if (!morse) continue;   // Skip unknown characters

        // Loop through each symbol (dot or dash) in the Morse code string
        for (int j = 0; morse[j] != '\0'; j++) {
            pwm_on();  // Turn on LED/sound to represent dot or dash

            // Delay based on symbol: short for dot, long for dash
            delay(morse[j] == '.' ? DOT_DURATION : DASH_DURATION);

            pwm_off();  // Turn off LED/sound after symbol

            // Add a short space between symbols within the same letter
            if (morse[j+1] != '\0') {
                delay(SYMBOL_SPACE);
            }
        }

        // Add a longer space between letters
        delay(LETTER_SPACE);
    }
}
