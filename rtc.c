/*
 *  rtc time is only fetched once by rtc_init (rtc_init should be called once)
 *  During the rest of the program the time is internally counted on an 1Hz Signal
 *  of the rtc.
 */

#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include "rtc.h"

#ifdef DEBUG
volatile uint8_t debug_count_A = 0;
volatile uint8_t debug_count_B = 0;
#endif

/* TWI Defines */
#define TWI_HANDLER_SET_TIME 1
#define TWI_HANDLER_SET_CONFIG_AND_GET_TIME 7

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

/* DST transform tables*/
const int8_t is_dst_table[] PROGMEM = {23, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22};
const int8_t is_no_dst_table[] PROGMEM = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 0};

/* Global variables */
volatile static time local_time;
volatile static uint8_t flag_second_tick = 0;

volatile static uint8_t set_raw_second = 0;
volatile static uint8_t set_raw_minute = 0;
volatile static uint8_t set_raw_hour = 0; 


volatile static uint8_t twi_handler_state = 0;
volatile static uint8_t flag_get_time = 0;

volatile static uint8_t is_dst = 0;
volatile static uint8_t dst_toggled = 0;

volatile static uint8_t button_already_pressed = 0;

/* EEPROM variables */
// eeprom_is_dst stores if the time value hold by the rtc reflects dst
// e.g. if the rtc time was last updated in summer
static uint8_t eeprom_is_dst EEMEM;
// rtc_dst_toggle stores if the time hold by the rtc needs to be shifted by one hour
// respecting eeprom_is_dst
static uint8_t eeprom_dst_toggled EEMEM;

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

    /* TWI Config */
    TWBR = 0x01;																//	\ 400000Hz SCL
	TWSR = (1<<TWPS0);															//	/

    /* ?? Reset ?? */

    /* RTC Send Config */
    twi_handler_state = TWI_HANDLER_SET_CONFIG_AND_GET_TIME;
    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN) | (1<<TWIE);

    eeprom_busy_wait();
    is_dst = eeprom_read_byte(&eeprom_is_dst);
    eeprom_busy_wait();
    dst_toggled = eeprom_read_byte(&eeprom_dst_toggled);

}

/* check if a second has passed since the last call of rtc_get_time */
uint8_t rtc_is_second(void) {
    return flag_second_tick;
}

/* get local time */
time rtc_get_time(void) {   // inline?
    flag_second_tick = 0;
    if(dst_toggled) {
        if(is_dst) {
            return (time) {.hour = pgm_read_byte(&is_dst_table[local_time.hour]), 
                        .minute = local_time.minute,
                        .second = local_time.second };
        } else {
            return (time) {.hour = pgm_read_byte(&is_no_dst_table[local_time.hour]), 
                        .minute = local_time.minute,
                        .second = local_time.second };
        }
    } else {
        return local_time;
    }
}

uint8_t rtc_get_dst(void) {
    return is_dst;
}



/* set local and rtc time */
void rtc_set_time(time t, uint8_t dst) {
    set_raw_second = ((t.second / 10) << 4) | ((t.second % 10) & 0x0f); 
    set_raw_minute = ((t.minute / 10) << 4) | ((t.minute % 10) & 0x0f); 
    set_raw_hour = ((t.hour / 10) << 4) | ((t.hour % 10) & 0x0f); 
    
    
    EIMSK &= ~(1<<INT0);

    /* send start condition */
    twi_handler_state = TWI_HANDLER_SET_TIME;
    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN) | (1<<TWIE);

    /* set local time */
    local_time = t;

    is_dst = dst;
    eeprom_busy_wait();
    eeprom_write_byte(&eeprom_is_dst, dst);
    
    dst_toggled = 0;
    eeprom_busy_wait();
    eeprom_write_byte(&eeprom_dst_toggled, 0);
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

ISR(PCINT0_vect) {
    if (PINB & (1<<PIN4)) {
        if (button_already_pressed) return;
        button_already_pressed = 1;

        dst_toggled &= 1;
        dst_toggled ^= 1;
        eeprom_busy_wait();
        eeprom_write_byte(&eeprom_dst_toggled, dst_toggled);
        
        TCNT0 = 0;
        TCCR0B = (1<<CS02) | (1<<CS00);
    }
}

ISR(TIMER0_OVF_vect) {
    button_already_pressed = 0;
    TCCR0B = 0; 
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
        case TWI_HANDLER_SET_CONFIG_AND_GET_TIME + 0:
            /* send device address and set write mode */
            TWDR = (RTC_DEVICE_ADDRESS << 1) + TW_WRITE;
            TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            break;
        case TWI_HANDLER_SET_CONFIG_AND_GET_TIME + 1:
            /* send memory address to write to */
            TWDR = RTC_CONFIG_OFFSET;
		    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            break;
        case TWI_HANDLER_SET_CONFIG_AND_GET_TIME + 2:
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
        
        case TWI_HANDLER_SET_CONFIG_AND_GET_TIME + 3:
            /* send device address and set write mode */
            TWDR = (RTC_DEVICE_ADDRESS << 1) + TW_WRITE;
	        TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            break;
        case TWI_HANDLER_SET_CONFIG_AND_GET_TIME + 4:
            /* send memory address to read from*/
            TWDR = RTC_TIME_OFFSET;
		    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            break;
        case TWI_HANDLER_SET_CONFIG_AND_GET_TIME + 5:
            /* send repeated start to switch to read mode */
            TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN) | (1<<TWIE);
            break;
        case TWI_HANDLER_SET_CONFIG_AND_GET_TIME + 6:
            /* send device address and set read mode */
            TWDR = (RTC_DEVICE_ADDRESS << 1) + TW_READ;
            TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            break;
        case TWI_HANDLER_SET_CONFIG_AND_GET_TIME + 7:
            /* receive seconds */
            TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE) | (1<<TWEA);
            break;
        case TWI_HANDLER_SET_CONFIG_AND_GET_TIME + 8:
            /* read seconds from buffer and receive minutes */ 
            tmp_second = TWDR;
            TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE) | (1<<TWEA);
            break;
        case TWI_HANDLER_SET_CONFIG_AND_GET_TIME + 9:
            /* read minutes from buffer and receive hours */ 
            tmp_minute = TWDR;
            TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);                                  // last byte to be received hence no ack is send
            break;
        case TWI_HANDLER_SET_CONFIG_AND_GET_TIME + 10:
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