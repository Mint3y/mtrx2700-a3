#ifndef MORSE_H
#define MORSE_H

#include <stdint.h>

// Call this to flash a string in Morse code via TIM2->CCR1
void play_morse(const char *message);

#endif // MORSE_H
