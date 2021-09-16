#include <avr/interrupt.h>
#include "uart.h"
#include "binary_display.h"

// Command Definitions
#define NO_COMMAND 0x80
#define SET_RECEIVED_TIME 0x81
#define RECEIVE_TIME 0x82
#define RECEIVE_AND_SET_COLOR 0x83

static volatile union {
	struct {
		time t;
		uint8_t dst;
	};
	uint8_t byte[sizeof(time) + 1];
} new_time;

static volatile union {
	struct {
		color h, m, s, b;
	};
	uint8_t byte[sizeof(color)*4];
} new_colors;

FILE* uart_init(const uint32_t baudrate) {
	sei();
	static FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
	// stdout=&uart_output; // für printf
	UBRR0 = ((F_CPU/16/baudrate) - 1);
	UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);
	UCSR0C = (3<<UCSZ00)|(1<<UCSZ01);
	return &uart_output;
}

int uart_putchar(char c, FILE *stream) {
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
	return 0;
}

ISR(USART_RX_vect) {
	static uint8_t get_time_state = 0;
	static uint8_t i = 0;
	static uint8_t command = NO_COMMAND;
  
	DDRB &= ~(1<<PIN5);
	PORTB &= ~(1<<PIN5);

	uint8_t tmp = UDR0;
	if(tmp & (1<<7)) {
		// Command received
		command = tmp;
		
		if(command == SET_RECEIVED_TIME) {
			rtc_set_time(new_time.t, new_time.dst);
		}

		i = 0;
		return;
	}
	
	// data received
	uint8_t data = tmp;

	switch(command) {
	case RECEIVE_TIME:
		if(i < sizeof(new_time)) {
			new_time.byte[i] = data;
			i++;
		}
		if(i >= sizeof(new_time)) {
			// last time byte received
			i = 0;
			command = NO_COMMAND;
		}
		break;
	case RECEIVE_AND_SET_COLOR:
		if(i < sizeof(new_colors)) {
			new_colors.byte[i] = (data << 1) + 0x01;
			i++;
		}
		if(i >= sizeof(new_colors)) {
			// last color byte received
			bd_set_colors(new_colors.h, new_colors.m, new_colors.s, new_colors.b);
			i = 0;
			command = NO_COMMAND;
		}
		break;
	default: 
		break;	
	}
}
