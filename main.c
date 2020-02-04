#define F_CPU 16000000

#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include "rtc.h"
#include "uart.h"
#include "binary_display.h"


int main(void) {
  
    FILE* uart0 = uart_init(1000000);
    
    // fprintf(uart0, "\nStartup\n");
    
    // struct time t = get_time();
    // set_time((struct time){.second = 0, .minute = 10, .hour = 0});
    //  fprintf(uart0, "time: %i:%i:%i\r\n", t.hour, t.minute, t.second);

    // t = get_time();
    // fprintf(uart0, "time: %i:%i:%i\r\n", t.hour, t.minute, t.second);
   
    uint8_t seconds;
    while(1) {
        //struct time* new_t = update_time();
        
        
         //= {.second = 33, .minute = 44, .hour = 13};
        // set_time(t);
        // struct time* new_time = update_time();
        // if(new_time) {
        //     set_time((struct time){.second = 33, .minute = 44, .hour = 13});
        //     DDRB |= (1<<PIN5);
        //     PORTB |= (1<<PIN5);
        // }

        struct time t = get_time();
        
        if(t.second != seconds) {
            seconds = t.second;
            
            set_binary_display(t);
            

            fprintf(uart0, "time: %i:%i:%i\r\n", t.hour, t.minute, t.second);      
        }
    }
    return 0;
}