#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdint.h>

// Enables USART1 RX interrupt
void enable_interrupt(void);

// Called when a message is received (optional to make this global if reused)
void finished_transmission(uint8_t *user_buffer, uint32_t message_length);

#endif // INTERRUPT_H
