#include <stdint.h>
#include "stm32f303xc.h"
#include "serial.h"
#include "interrupt.h"

#define MESSAGE_LENGTH 100  // Defining message length

uint8_t kernel_buffer[2][MESSAGE_LENGTH];  // Double buffer system
uint8_t active_kernel = 0;                 // Tracks which buffer is active
uint8_t kernel_index = 0;                  // Current position in active buffer
char message_terminator = '\r';            // Terminating character

uint8_t transmit_buffer[MESSAGE_LENGTH];   // Holds data to be transmitted
uint8_t transmit_index = 0;                // Buffer position
uint8_t transmit_length = 0;               // Total number of characters
uint8_t is_transmitting = 0;               // Variable to track whether we are transmitting


void finished_transmission(uint8_t *user_buffer, uint32_t message_length) {
    user_buffer[message_length] = '\0';  // Null terminate
    // Copy the message into the transmit buffer
    for (uint32_t i = 0; i <= message_length; i++) {
        transmit_buffer[i] = user_buffer[i];
    }
    transmit_length = message_length;  // Set how many characters to send
    transmit_index = 0;                // Start from beginning
    is_transmitting = 1;               // Set transmit mode

    USART1->CR1 |= USART_CR1_TXEIE;    // Enable transmit interrupt to start sending
}

void USART1_IRQHandler(void)
{
    if (USART1->ISR & USART_ISR_RXNE) {  // Check if data has been received
        uint8_t incoming_byte = USART1->RDR;  // Read the character

        // Check for buffer overflow
        if (kernel_index < MESSAGE_LENGTH - 1) {
            // Store character in current buffer
            kernel_buffer[active_kernel][kernel_index++] = incoming_byte;

            // Check if terminating character
            if (incoming_byte == message_terminator) {
                // Callback function
                finished_transmission(kernel_buffer[active_kernel], kernel_index);

                // Switch to the other buffer for the next message
                active_kernel ^= 1;

                // Reset index for the new message
                kernel_index = 0;
            }
        } else {
            // Reset buffer if overflow
            kernel_index = 0;
        }
    }

    // Check if ready to send the next character and currently transmitting
    if ((USART1->ISR & USART_ISR_TXE) && is_transmitting) {
        if (transmit_index < transmit_length) {
            USART1->TDR = transmit_buffer[transmit_index++];  // Send next character
        } else {
            USART1->CR1 &= ~USART_CR1_TXEIE;  // Stop TX interrupt
            is_transmitting = 0;              // Stop transmit mode
        }
    }
}

void enable_interrupt() {
    __disable_irq();  // Disable interrupts

    USART1->CR1 |= USART_CR1_RXNEIE;      // Enable RX interrupt
    NVIC_SetPriority(USART1_IRQn, 1);     // Set priority for USART1 interrupt
    NVIC_EnableIRQ(USART1_IRQn);          // Enable USART1 interrupt in NVIC

    __enable_irq();   // Re-enable interrupts
}
