#include <avr/io.h>
#include <avr/interrupt.h>
#include "button.h"
#include "binary_display.h"

volatile static uint8_t button_already_pressed = 0;
volatile static uint8_t button_pressed_for_first_time = 1;

void button_init() {
	sei();

	/* PCINT4 Config (Button) */
	MCUCR &= ~(1<<PUD);
	DDRB &= ~(1<<PIN4);
	PORTB |= (1<<PIN4);
	PCICR = (1<<PCIE0);
	PCMSK0 = (1<<PCINT4);

	/* Timer0 Config (Button cooldown) */
	TCCR0A = 0;
	TCCR0B = 0; //(1<<CS02) | (1<<CS00);
	TIMSK0 = (1<<TOIE0);
	TCNT0 = 0;
}

ISR(PCINT0_vect) {
	if (PINB & (1<<PIN4)) {
		if (button_already_pressed) return;
		button_already_pressed = 1;
		
		if(button_pressed_for_first_time) {
			// First push of the button does not toggle dst but deactivates low current mode
			button_pressed_for_first_time = 0;
			bd_set_low_current_mode(0);				
		} else {
			// Every following push, will toggle dst
			rtc_toggle_dst();
		}	

		TCNT0 = 0;
		TCCR0B = (1<<CS02) | (1<<CS00);
	}
}

ISR(TIMER0_OVF_vect) {
	button_already_pressed = 0;
	TCCR0B = 0; 
}
