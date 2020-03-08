#include "rtc.h" 

static volatile uint8_t r_raw_seconds;
static volatile uint8_t r_raw_minutes;
static volatile uint8_t r_raw_hours;

static volatile uint8_t w_raw_seconds;
static volatile uint8_t w_raw_minutes;
static volatile uint8_t w_raw_hours;

static volatile uint8_t state = 0;
static volatile uint8_t append_set_time_job = 0;


struct time get_time(void) {
    sei();
    /* transform raw seconds, etc. */
    struct time t;
    t.second = (r_raw_seconds >> 4) * 10 + (r_raw_seconds & 0x0f);
    t.minute = (r_raw_minutes >> 4) * 10 + (r_raw_minutes & 0x0f);
    t.hour = ((r_raw_hours >> 4) & 0x03) * 10 + (r_raw_hours & 0x0f);
    
    if(!state) {                                                                    // only start, if there is not already a job running
        state = 1;
        /* configure TWI and send start condition */
        // TWBR = 0x01;
        // TWSR = 0x00;
        TWBR = 0x01;																//	\ 400000Hz SCL
	    TWSR = (1<<TWPS0);															//	/
	    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN) | (1<<TWIE);
    }

    return t;
}

void set_time(struct time t) {
    if(append_set_time_job) return;                                                 // trying to set rtc too fast
    sei();
    w_raw_seconds = ((t.second / 10) << 4) | ((t.second % 10) & 0x0f); 
    w_raw_minutes = ((t.minute / 10) << 4) | ((t.minute % 10) & 0x0f); 
    w_raw_hours = ((t.hour / 10) << 4) | ((t.hour % 10) & 0x0f); 
   
    if(state) {                                                                     // if a job is running, append the new one to the current
        append_set_time_job = 1;
    } else {
        state = 9;
        /* configure TWI and send start condition */
        TWBR = 0x01;
        TWSR = 0x00;
        
        // TWBR = 0x01;																//	\ 400000Hz SCL
	    // TWSR = (1<<TWPS0);															//	/
	    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN) | (1<<TWIE);
    }
}

ISR(TWI_vect) {
	
    static uint8_t r_raw_seconds_temp = 0;
    static uint8_t r_raw_minutes_temp = 0;

    switch(state) {

        /* READ TIME */
        case 1:
            /* send device address and set write mode */
            TWDR = (RTC_DEVICE_ADDRESS << 1) + TW_WRITE;
	        TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            break;
        case 2:
            /* send memory address to read from*/
            TWDR = RTC_MEMORY_START_ADDRESS;
		    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            break;
        case 3:
            /* send repeated start to switch to read mode */
            TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN) | (1<<TWIE);
            break;
        case 4:
            /* send device address and set read mode */
            TWDR = (RTC_DEVICE_ADDRESS << 1) + TW_READ;
            TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            break;
        case 5:
            /* receive seconds */
            TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE) | (1<<TWEA);
            break;
        case 6:
            /* read seconds from buffer and receive minutes */ 
            r_raw_seconds_temp = TWDR;
            TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE) | (1<<TWEA);
            break;
        case 7:
            /* read minutes from buffer and receive hours */ 
            r_raw_minutes_temp = TWDR;
            TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);                                  // last byte to be received hence no ack is send
            break;
        case 8:
            /* read hours from buffer and send stop condition if no set_time_job is pending*/
            r_raw_seconds = r_raw_seconds_temp;                                         // hold back rawsec and rawmin values till this point to avoid racing, if the 
            r_raw_minutes = r_raw_minutes_temp;                                         // get_time function is called faster than the twi operates
            r_raw_hours = TWDR;
            if(!append_set_time_job) {
                TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN) | (1<<TWIE);
                state = 0;
                return;                                                                 // return so state will not be incremented
            }
            
         /* WRITE TIME */
            /* if a set_time_job is pending, initiate it by sending a repeated start */
            TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN) | (1<<TWIE);
            break;
        case 9: 
            /* send device address and set read mode */
            TWDR = (RTC_DEVICE_ADDRESS << 1) + TW_WRITE;
            TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            break;
        case 10:
            /* send memory address to write to */
            TWDR = RTC_MEMORY_START_ADDRESS;
		    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            break;
        case 11:
            /* send seconds */
            TWDR = w_raw_seconds;
		    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            break;      
        case 12:
            /* send minutes */
            TWDR = w_raw_minutes;
		    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            break;      
        case 13:
            /* send hours */
            TWDR = w_raw_hours;
		    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            break;     
        case 14:
            /* send stop condition */
            TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN) | (1<<TWIE);
            state = 0;
            append_set_time_job = 0;        
            return;                                                             // return so state will not be incremented
        
        default: return;
    }
    state++;
}