#include <avr/interrupt.h>
#include "uart.h"

static volatile time new_time;

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
    
    DDRB &= ~(1<<PIN5);
    PORTB &= ~(1<<PIN5);

    uint8_t i = UDR0;
    if(i == 0xff) {
        get_time_state = 0;
        return;
    }
    switch(get_time_state) {
        case 0:
            new_time.hour = i;    
            break;
        case 1:
            new_time.minute = i;
            break;
        case 2:
            new_time.second = i;
            break;
        case 3:
            /*DST Byte ignored*/
            DDRB |= (1<<PIN5);
            PORTB |= (1<<PIN5);
            rtc_set_time(new_time, (date){0,0,0});
            get_time_state = 0xff;
            return;
        default: return;
    }
    get_time_state++;
}