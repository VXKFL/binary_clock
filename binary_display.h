#ifndef _BINARY_DISPLAY_H
#define _BINARY_DISPLAY_H

#include "rtc.h"

#pragma pack(1)

typedef struct {
    uint8_t green;
    uint8_t red;
    uint8_t blue;
} color;

void bd_set_time(time);
void bd_set_color_h(color h);
void bd_set_color_m(color m);
void bd_set_color_s(color s);
void bd_set_color_b(color b);
void bd_update_eemem(void);
void bd_init(void);
void bd_set_low_current_mode(uint8_t i);

#endif
