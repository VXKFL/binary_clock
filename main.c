#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include "rtc.h"
#include "uart.h"
#include "binary_display.h"

#ifdef DEBUG
extern volatile uint8_t debug_count_A;
extern volatile uint8_t debug_count_B;
#endif

int main(void) {
    
    rtc_init();
    FILE* uart0 = uart_init(1000000);

    while(1) {

#ifndef DEBUG   
        if(rtc_is_second()) {
            time t = rtc_get_time();
            fprintf(uart0, "time: %i:%i:%i, dst: %i\r\n", t.hour, t.minute, t.second, rtc_get_dst()); 
            set_binary_display(t);
        }
#else
        _delay_ms(20);
        fprintf(uart0, "A: %i B: %i time: %i:%i:%i\r\n", debug_count_A, debug_count_B, t.hour, t.minute, t.second);  
#endif    
    
    }
    return 0;
}