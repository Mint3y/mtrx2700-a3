#ifndef SERIAL_PORT_HEADER
#define SERIAL_PORT_HEADER

#include <stdint.h>
#include <stdbool.h>
#include "stm32f303xc.h"

#ifndef SERIAL_BUFFER_SIZE
#define SERIAL_BUFFER_SIZE 1024
#endif // SERIAL_BUFFER_SIZE

#ifndef SERIAL_TERMINATOR
#define SERIAL_TERMINATOR '*'
#endif // SERIAL_TERMINATOR

#define BASE_CLOCK (8000000) // 8MHz

// Serial buffer capable of being used for both receiving and transmitting
typedef struct _SerialPortBuffer {        // Used for receiving      / Used for transmitting
	char buffer[SERIAL_BUFFER_SIZE];      // Buffer to read to       / Buffer to write from
	struct _SerialPortBuffer* buffer_ref; // NOT USED                / Buffer reference
	uint32_t index;                       // Number of bytes read    / Number of bytes written
	uint32_t length;                      // Length of buffer (1024) / Length to write
	bool ready;                           // Buffer is not reading   / Buffer is not writing
} SerialPortBuffer;

// We store the pointers to the GPIO and USART that are used
//  for a specific serial port. To add another serial port
//  you need to select the appropriate values.
typedef struct _SerialPort {
	USART_TypeDef *UART;
	GPIO_TypeDef  *GPIO;
	volatile uint32_t MaskAPB2ENR;	// mask to enable RCC APB2 bus registers
	volatile uint32_t MaskAPB1ENR;	// mask to enable RCC APB1 bus registers
	volatile uint32_t MaskAHBENR;	// mask to enable RCC AHB bus registers
	volatile uint32_t SerialPinModeValue;
	volatile uint32_t SerialPinSpeedValue;
	volatile uint32_t SerialPinAlternatePinValueLow;
	volatile uint32_t SerialPinAlternatePinValueHigh;
	void (*read_complete)(SerialPortBuffer*); // When executed this function 
                                              // should mark buffer as ready
                                              // for future use
	void (*write_complete)(SerialPortBuffer*); // Executed at end of transmission
} SerialPort;

enum BaudRate {
  BAUD_9600,
  BAUD_19200,
  BAUD_38400,
  BAUD_57600,
  BAUD_115200
};

// Initialises the serial module. Should be called before using any serial
// functions.
void init_serial();

// Enables clock power and system configuration clock. TODO move to init.h
void init_usart();

// Initialises a serial port.
// baud_rate:           Port baud rate as specified by an enum (TODO: change)
// serial_port:         Port register information
// completion_function: Function to execute when completing serial output,
//                      takes in the number of bytes sent.
void init_serial_port_16bit(enum BaudRate baud_rate,
                            SerialPort*   serial_port,
                            void (*read_complete)(SerialPortBuffer*));

// Enables interrupt requests for USART1.
void enable_usart1_interrupts();

// Enables triggering interrupts when USART1 receives a byte.
void enable_usart1_receive_interrupt();

// Enables triggering interrupts when USART1 transmits a byte.
void enable_usart1_transmit_interrupt();

// Disables triggering interrupts when USART1 transmits a byte.
void disable_usart1_transmit_interrupt();

// Get the module's USART1 Serial Port.
// Returns the module's USART1 Serial Port
SerialPort* get_usart1_port();

// Gets a pointer to the next open transmit buffer. A transmission buffer is 
// considered open if it is not filled nor immediately transmitting. Marks the
// obtained buffer as unready (i.e closed).
// Returns the open transmit buffer or NULL if none were available
SerialPortBuffer* get_open_transmit_buffer();

// Sets the transmit completion function for the USART1 Port
// write_completion: The completion function
void set_write_completion(void write_completion(SerialPortBuffer*));

// Begins transmission of the string stored in the ready transmission buffer.
// Via USART1
void begin_transmit_ready();

// Attempt to transmit a string. Fails silently if no transmission buffer is available.
// data:   The string
// length: The length of the string
void attempt_serial_transmit(char* data, uint32_t length);

// Default callback for write completion
// buf: The transmit buffer that finished transmitting
void default_write_completion(SerialPortBuffer* buf);

// Naive polling implementation of serial read.
// terminator:  The terminating character to read until
// buf:         The buffer to read into
// serial_port: The port to read from
int serial_read_until_terminator(uint8_t           terminator,
                                 SerialPortBuffer* buf,
                                 SerialPort*       serial_port);

// Naively writes a char to a serial port. Waits for transmit buffer to be
// empty with a while loop.
// data: 		The char to write
// serial_port: The port to write to
void serial_write_char(char data, SerialPort* serial_port);

// Naively writes a string to a serial port.
// data:        The string to write
// length:      The length of the string
// serial_port: The port to write to
void serial_write_string(char* data, uint32_t length, SerialPort* serial_port);

// Polling based serial example
void test_serial();

// Interrupt based serial example
void test_serial_interrupt();

#endif // SERIAL_PORT_HEADER
