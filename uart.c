#include "uart.h"

static volatile struct time new_time;

FILE* uart_init(unsigned long baudrate) {
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

// struct time* update_time(void) {
//     if(new_time_available) {
//         UCSR0B &= ~(1<<RXCIE0);
//         new_time_available = 0;
//         new_time = new_time_buf;
//         UCSR0B |= (1<<RXCIE0);
//         return &new_time;    
//     }
//     return NULL;
// }

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
            set_time(new_time);
            get_time_state = 0xff;
            return;
        default: return;
    }
    get_time_state++;
}