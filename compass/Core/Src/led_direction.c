#include "led_direction.h"
#include "main.h"
#include "led.h"


void turn_off_all_leds(void) {
	set_led_register(0x00);
}

Direction get_direction_from_heading(float heading) {
    if (heading >= 337.5 || heading < 22.5) return DIR_N;
    else if (heading >= 22.5 && heading < 67.5) return DIR_NE;
    else if (heading >= 67.5 && heading < 112.5) return DIR_E;
    else if (heading >= 112.5 && heading < 157.5) return DIR_SE;
    else if (heading >= 157.5 && heading < 202.5) return DIR_S;
    else if (heading >= 202.5 && heading < 247.5) return DIR_SW;
    else if (heading >= 247.5 && heading < 292.5) return DIR_W;
    else if (heading >= 292.5 && heading < 337.5) return DIR_NW;
    return DIR_NONE;
}

void turn_on_direction_led(Direction dir) {
	uint8_t led_state = 0x00;

	    switch (dir) {
	        case DIR_N:  led_state = 0b00000010; break; // LD3 = PE9 (bit 1)
	        case DIR_NE: led_state = 0b00000100; break; // LD5 = PE10 (bit 2)
	        case DIR_E:  led_state = 0b00001000; break; // LD7 = PE11 (bit 3)
	        case DIR_SE: led_state = 0b00010000; break; // LD9 = PE12 (bit 4)
	        case DIR_S:  led_state = 0b00100000; break; // LD10 = PE13 (bit 5)
	        case DIR_SW: led_state = 0b01000000; break; // LD8 = PE14 (bit 6)
	        case DIR_W:  led_state = 0b10000000; break; // LD6 = PE15 (bit 7)
	        case DIR_NW: led_state = 0b00000001; break; // LD4 = PE8 (bit 0)
	        default:     led_state = 0x00; break;
	    }

	    set_led_register(led_state);
}
