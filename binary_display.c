#include "binary_display.h"

static color img_buf[27] = {0}; 
static color color_h;
static color color_m;
static color color_s;
static color color_b;

// EEPROM variables
static color color_h_eemem EEMEM;
static color color_m_eemem EEMEM;
static color color_s_eemem EEMEM;
static color color_b_eemem EEMEM;

static uint8_t flag_update_eemem = 0;

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
	if(!flag_update_eemem) return;
	eeprom_busy_wait();
	eeprom_update_block(&color_h,  &color_h_eemem, sizeof(color));	
	eeprom_busy_wait();
	eeprom_update_block(&color_m,  &color_m_eemem, sizeof(color));	
	eeprom_busy_wait();
	eeprom_update_block(&color_s,  &color_s_eemem, sizeof(color));	
	eeprom_busy_wait();
	eeprom_update_block(&color_b,  &color_b_eemem, sizeof(color));	

	flag_update_eemem = 0;
}



void bd_set_colors(color h, color m, color s, color b) {
	color_h = h;
	color_m = m;
	color_s = s;
	color_b = b;
	flag_update_eemem = 1;
}

void bd_set_time(time t) {
    for(int i = 0; i < 5; i++) {
        if(t.hour & 0x01) {
            img_buf[22 + i] = color_h; //(color){0x20,0x20,0x20}; // color_h; 
        } else {
            img_buf[22 + i] = (color){0};
        }
        t.hour >>= 1;
    }

    for(int i = 0; i < 6; i++) {
        if(t.minute & 0x01) {
            img_buf[21 - i] = color_m; //(color){0x20,0x20,0x20}; // color_m;
        } else {
            img_buf[21 - i] = (color){0};
        }
        t.minute >>= 1;
    }

    for(int i = 0; i < 6; i++) {
        if(t.second & 0x01) {
            img_buf[10 + i] = color_s; //(color){0x20,0x20,0x20}; // color_s; 
        } else {
            img_buf[10 + i] = (color){0};
        }
        t.second >>= 1;
    }

    for(int i = 0; i < 10; i++) {
        img_buf[i] = color_b; //(color){0x20,0x20,0x20}; //color_b; 
    }

    // img_buf[3] = (color){0x00, 0x00, 0x00};

    sendbyte((void*) img_buf, 27 * sizeof(color));
}
