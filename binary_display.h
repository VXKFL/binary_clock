#ifndef _BINARY_DISPLAY_H
#define _BINARY_DISPLAY_H

#include <avr/io.h>
#include <avr/eeprom.h>
#include "rtc.h"
#include "sendbyte.h" 

#pragma pack(1)

typedef struct {
    uint8_t green;
    uint8_t red;
    uint8_t blue;
} color;

void bd_set_time(time);
void bd_set_colors(color h, color m, color s, color b);
void bd_update_eemem(void);
void bd_init(void);

#endif
