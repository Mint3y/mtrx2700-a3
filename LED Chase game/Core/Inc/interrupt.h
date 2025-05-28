#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdint.h>

#define MESSAGE_LENGTH 512
extern volatile uint8_t kernel_index;
extern volatile uint8_t active_kernel;
extern volatile uint8_t kernel_buffer[2][MESSAGE_LENGTH];

void enable_interrupt(void);
void finished_transmission(uint8_t *user_buffer, uint32_t message_length);
extern volatile uint8_t is_transmitting;
uint8_t serial_message_available(void);
void get_serial_message(char *buffer);



// These are the variables you need:


#endif // INTERRUPT_H
