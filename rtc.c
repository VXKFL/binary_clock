/*
 *  rtc time is only fetched once by rtc_init (rtc_init should be called once)
 *  During the rest of the program the time is internally counted on an 1Hz Signal
 *  of the rtc.
 */

#include <avr/interrupt.h>
#include "rtc.h"

#ifdef DEBUG
volatile uint8_t debug_count_A = 0;
volatile uint8_t debug_count_B = 0;
#endif

#define TWI_HANDLER_SET_TIME 1
#define TWI_HANDLER_SET_CONFIG_AND_TIME 7

/* RTC Patterns */
volatile const uint8_t rtc_config_pattern[] = {
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000
};

/* Global variables */
volatile static time local_time;
volatile static uint8_t flag_second_tick = 0;

volatile static uint8_t set_raw_second = 0;
volatile static uint8_t set_raw_minute = 0;
volatile static uint8_t set_raw_hour = 0; 

volatile static uint8_t twi_handler_state = 0;
volatile static uint8_t flag_get_time = 0;

/*
 *  Initialize the RTC
 *  This function should only be called once, at startup.
 *  Calling this function does not reset the time of the rtc. For setting the rtc time, use rtc_set_time.
 */
void rtc_init(void) {
    sei();

    /* INT0 Config */
    DDRD &= ~(1<<PIN2);
    PORTD |= (1<<PIN2);
    MCUCR &= ~(1<<PUD);
    EICRA |= (1<<ISC01); EICRA &= ~(1<<ISC00);
    EIMSK |= (1<<INT0);

    /* TWI Config */
    TWBR = 0x01;																//	\ 400000Hz SCL
	TWSR = (1<<TWPS0);															//	/

    /* ?? Reset ?? */

    /* RTC Send Config */
    twi_handler_state = 7;
    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN) | (1<<TWIE);
    
}

/* check if a second has passed since the last call of rtc_get_time */
uint8_t rtc_is_second(void) {
    return flag_second_tick;
}

/* get local time */
time rtc_get_time(void) {   // inline?
    flag_second_tick = 0;
    return local_time;
}

/* set local and rtc time */
void rtc_set_time(time t, date d) {
    set_raw_second = ((t.second / 10) << 4) | ((t.second % 10) & 0x0f); 
    set_raw_minute = ((t.minute / 10) << 4) | ((t.minute % 10) & 0x0f); 
    set_raw_hour = ((t.hour / 10) << 4) | ((t.hour % 10) & 0x0f); 

    EIMSK &= ~(1<<INT0);

    /* send start condition */
    twi_handler_state = 1;
    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN) | (1<<TWIE);

    /* set local time */
    local_time = t;
}

ISR(INT0_vect) {
#ifdef DEBUG    
    debug_count_A++;
#endif
    local_time.second++;
    if(local_time.second > 59) { local_time.second = 0; local_time.minute++; }
    if(local_time.minute > 59) { local_time.minute = 0; local_time.hour++; }
    if(local_time.hour > 23) { local_time.hour = 0; }
        
    if(flag_get_time) {
        flag_get_time = 0;
        TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN) | (1<<TWIE);
    }
    flag_second_tick = 1;
}

ISR(TWI_vect) {
    static uint8_t tmp_iterate = 0;
    static uint8_t tmp_second = 0;
    static uint8_t tmp_minute = 0;

    switch(twi_handler_state) {
        /* SET TIME */
        case TWI_HANDLER_SET_TIME + 0:
            /* send device address and set write mode */
            TWDR = (RTC_DEVICE_ADDRESS << 1) + TW_WRITE;
            TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            break;
        case TWI_HANDLER_SET_TIME + 1:
            /* send memory address to write to */
            TWDR = RTC_TIME_OFFSET;
		    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            break;
        case TWI_HANDLER_SET_TIME + 2:
            /* send seconds */
            TWDR = set_raw_second;
		    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            break;      
        case TWI_HANDLER_SET_TIME + 3:
            /* send minutes */
            TWDR = set_raw_minute;
		    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            break;      
        case TWI_HANDLER_SET_TIME + 4:
            /* send hours */
            TWDR = set_raw_hour;
		    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            break;     
        case TWI_HANDLER_SET_TIME + 5:
            /* send stop condition */
            TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN) | (1<<TWIE);
            twi_handler_state = 0;
            EIFR |= (1<<INTF0);
            EIMSK |= (1<<INT0);
            return;                                                             // return so state will not be incremented

        /* SET CONFIG & GET TIME */
        case TWI_HANDLER_SET_CONFIG_AND_TIME + 0:
            /* send device address and set write mode */
            TWDR = (RTC_DEVICE_ADDRESS << 1) + TW_WRITE;
            TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            break;
        case TWI_HANDLER_SET_CONFIG_AND_TIME + 1:
            /* send memory address to write to */
            TWDR = RTC_CONFIG_OFFSET;
		    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            break;
        case TWI_HANDLER_SET_CONFIG_AND_TIME + 2:
            /* send config */
            if( tmp_iterate < sizeof(rtc_config_pattern) / sizeof(uint8_t) ) {
                TWDR = rtc_config_pattern[tmp_iterate++];
	            TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
                return;
            }
            tmp_iterate = 0;
            /* send stop condition */
            TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN) | (1<<TWIE);
            flag_get_time = 1;
            break;
        
        case TWI_HANDLER_SET_CONFIG_AND_TIME + 3:
            /* send device address and set write mode */
            TWDR = (RTC_DEVICE_ADDRESS << 1) + TW_WRITE;
	        TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            break;
        case TWI_HANDLER_SET_CONFIG_AND_TIME + 4:
            /* send memory address to read from*/
            TWDR = RTC_TIME_OFFSET;
		    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            break;
        case TWI_HANDLER_SET_CONFIG_AND_TIME + 5:
            /* send repeated start to switch to read mode */
            TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN) | (1<<TWIE);
            break;
        case TWI_HANDLER_SET_CONFIG_AND_TIME + 6:
            /* send device address and set read mode */
            TWDR = (RTC_DEVICE_ADDRESS << 1) + TW_READ;
            TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            break;
        case TWI_HANDLER_SET_CONFIG_AND_TIME + 7:
            /* receive seconds */
            TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE) | (1<<TWEA);
            break;
        case TWI_HANDLER_SET_CONFIG_AND_TIME + 8:
            /* read seconds from buffer and receive minutes */ 
            tmp_second = TWDR;
            TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE) | (1<<TWEA);
            break;
        case TWI_HANDLER_SET_CONFIG_AND_TIME + 9:
            /* read minutes from buffer and receive hours */ 
            tmp_minute = TWDR;
            TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);                                  // last byte to be received hence no ack is send
            break;
        case TWI_HANDLER_SET_CONFIG_AND_TIME + 10:
            /* read hours from buffer, convert and send stop condition */
            local_time.second = (tmp_second >> 4) * 10 + (tmp_second & 0x0f);
            local_time.minute = (tmp_minute >> 4) * 10 + (tmp_minute & 0x0f);
            local_time.hour = ((TWDR >> 4) & 0x03) * 10 + (TWDR & 0x0f);

            /* send stop condition */
            TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN) | (1<<TWIE);
            twi_handler_state = 0;
            return;      

        default: return;
    }
    twi_handler_state++;
}