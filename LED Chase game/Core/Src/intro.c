#include "stm32f303xc.h"
#include <string.h>

void print_intro(void) {
	const char *message_lines[] = {
	    "Welcome to Treasure Hunter Training!",
	    "",
	    "You are about to begin a series of reflex and decision making trials.",
	    "Use your speed, your memory, and your instincts to overcome them.",
	    "Complete all challenges to officially become a Treasure Hunter!",
	    "",
	    "Your first challenge is the LED Chase Game.",
	    "LEDs will bounce left and right rapidly across a row.",
	    "A target LED will flash at the start,remember its position!",
	    "Then, press the button at the exact moment the bouncing light lands on the target.",
	    "Reflexes are everything",
	    "",
	    "There are 3 levels of increasing difficulty. Each will get faster.",
	    "You must pass all 3 levels to receive your next challenge",
	    "",
	    "Press the blue button to begin your journey!~"  // <-- end-of-message marker
	};

    for (int i = 0; i < sizeof(message_lines) / sizeof(message_lines[0]); i++) {
        const char *line = message_lines[i];
        const char *to_send = (line[0] == '\0') ? "\r\n" : line;

        for (int j = 0; j < strlen(to_send); j++) {
            while (!(USART1->ISR & USART_ISR_TXE)); // Wait until TX buffer is empty
            USART1->TDR = to_send[j];
        }

        while (!(USART1->ISR & USART_ISR_TC));  // Wait until transmission complete

    }
}

void print_morse_puzzle(void) {
    const char *message_lines[] = {
        "You can hear me, but I do not speak.\r",
        "Your secrets, safely I will keep.\r",
        "",
        "I have travelled far, across many oceans and lands.\r",
        "I dance to a rhythm that few understand.\r",
        "",
        "A cipher of light or sound, a language in disguise,\r",
        "A code of pulses, sent among spies.\r",
        "",
        "My skin, dashed with lines, bearing stories gone by.\r",
        "My eyes trace scattered dots, where hidden messages lie.\r",
        "",
        "What am I?\r\n~"
    };

    for (int i = 0; i < sizeof(message_lines) / sizeof(message_lines[0]); i++) {
        const char *line = message_lines[i];
        const char *to_send = (line[0] == '\0') ? "\r\n" : line;

        for (int j = 0; j < strlen(to_send); j++) {
            while (!(USART1->ISR & USART_ISR_TXE));
            USART1->TDR = to_send[j];
        }

        while (!(USART1->ISR & USART_ISR_TC));

    }
}

void print_correct_response(void) {
    const char *message_lines[] = {
        "Correct! You have solved the riddle.\r",
        "",
        "Can you translate this message to find the next challenge\r",
		"Press the blue button to try!\r~"// Ending with '~' ensures GUI update
    };

    for (int i = 0; i < sizeof(message_lines) / sizeof(message_lines[0]); i++) {
        const char *line = message_lines[i];
        const char *to_send = (line[0] == '\0') ? "\r\n" : line;

        for (int j = 0; j < strlen(to_send); j++) {
            while (!(USART1->ISR & USART_ISR_TXE));
            USART1->TDR = to_send[j];
        }

        while (!(USART1->ISR & USART_ISR_TC));
    }
}
