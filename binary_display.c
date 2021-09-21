#include <avr/io.h>
#include <avr/eeprom.h>
#include "binary_display.h"
#include "sendbyte.h" 

static color img_buf[27] = {0}; 
static color color_h;
static color color_m;
static color color_s;
static color color_b;

static uint8_t low_current_mode = 1;
#define LOW_CURRENT_DIVISOR 3

// EEPROM variables
static color color_h_eemem EEMEM;
static color color_m_eemem EEMEM;
static color color_s_eemem EEMEM;
static color color_b_eemem EEMEM;

static uint8_t flag_update_eemem_color_h;
static uint8_t flag_update_eemem_color_m;
static uint8_t flag_update_eemem_color_s;
static uint8_t flag_update_eemem_color_b;

void bd_init(void) {
	eeprom_busy_wait();
	eeprom_read_block(&color_h,  &color_h_eemem, sizeof(color));	
	eeprom_busy_wait();
	eeprom_read_block(&color_m,  &color_m_eemem, sizeof(color));	
	eeprom_busy_wait();
	eeprom_read_block(&color_s,  &color_s_eemem, sizeof(color));	
	eeprom_busy_wait();
	eeprom_read_block(&color_b,  &color_b_eemem, sizeof(color));	
}

void bd_update_eemem(void) {
	if(flag_update_eemem_color_h) { 
		eeprom_busy_wait();
		eeprom_update_block(&color_h,  &color_h_eemem, sizeof(color));	
		flag_update_eemem_color_h = 0;
	}
	if(flag_update_eemem_color_m) { 
		eeprom_busy_wait();
		eeprom_update_block(&color_m,  &color_m_eemem, sizeof(color));	
		flag_update_eemem_color_m = 0;
	}
	if(flag_update_eemem_color_s) { 
		eeprom_busy_wait();
		eeprom_update_block(&color_s,  &color_s_eemem, sizeof(color));	
		flag_update_eemem_color_s = 0;
	}
	if(flag_update_eemem_color_b) { 
		eeprom_busy_wait();
		eeprom_update_block(&color_b,  &color_b_eemem, sizeof(color));	
		flag_update_eemem_color_b = 0;
	}
}



void bd_set_color_h(color h) {
	color_h = h;
	flag_update_eemem_color_h = 1;
}

void bd_set_color_m(color m) {
	color_m = m;
	flag_update_eemem_color_m = 1;
}

void bd_set_color_s(color s) {
	color_s = s;
	flag_update_eemem_color_s = 1;
}

void bd_set_color_b(color b) {
	color_b = b;
	flag_update_eemem_color_b = 1;
}

// On startup low current mode is active (1)
void bd_set_low_current_mode(uint8_t i) {
	low_current_mode = i;
}

void bd_set_time(time t) {
	color h, m, s, b;
	if(low_current_mode) {
		h.red = color_h.red / LOW_CURRENT_DIVISOR;
		h.green = color_h.green / LOW_CURRENT_DIVISOR;
		h.blue = color_h.blue / LOW_CURRENT_DIVISOR;

		m.red = color_m.red / LOW_CURRENT_DIVISOR;
		m.green = color_m.green / LOW_CURRENT_DIVISOR;
		m.blue = color_m.blue / LOW_CURRENT_DIVISOR;

		s.red = color_s.red / LOW_CURRENT_DIVISOR;
		s.green = color_s.green / LOW_CURRENT_DIVISOR;
		s.blue = color_s.blue / LOW_CURRENT_DIVISOR;

		b.red = color_b.red / LOW_CURRENT_DIVISOR;	
		b.green = color_b.green / LOW_CURRENT_DIVISOR;	
		b.blue = color_b.blue / LOW_CURRENT_DIVISOR;	
	} else {	
		h = color_h;
		m = color_m;
		s = color_s;
		b = color_b;
	}	

    for(int i = 0; i < 5; i++) {
        if(t.hour & 0x01) {
            img_buf[22 + i] = h; //(color){0x20,0x20,0x20}; // color_h; 
        } else {
            img_buf[22 + i] = (color){0};
        }
        t.hour >>= 1;
    }

    for(int i = 0; i < 6; i++) {
        if(t.minute & 0x01) {
            img_buf[21 - i] = m; //(color){0x20,0x20,0x20}; // color_m;
        } else {
            img_buf[21 - i] = (color){0};
        }
        t.minute >>= 1;
    }

    for(int i = 0; i < 6; i++) {
        if(t.second & 0x01) {
            img_buf[10 + i] = s; //(color){0x20,0x20,0x20}; // color_s; 
        } else {
            img_buf[10 + i] = (color){0};
        }
        t.second >>= 1;
    }

    for(int i = 0; i < 10; i++) {
        img_buf[i] = b; //(color){0x20,0x20,0x20}; //color_b; 
    }

    // img_buf[3] = (color){0x00, 0x00, 0x00};

    sendbyte((void*) img_buf, 27 * sizeof(color));
}
