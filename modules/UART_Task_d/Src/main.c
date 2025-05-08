#include "serial.h"
#include "interrupt.h"
#include "stm32f303xc.h"

int main(void)
{
    SerialInitialise(BAUD_115200, &USART1_PORT, 0x00);  // Initialise USART1
    enable_interrupt();                                 // Enable interrupts

    for (;;); // Infinite loop
}
