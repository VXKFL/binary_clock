#ifndef _BINARY_DISPLAY_H
#define _BINARY_DISPLAY_H

#include <avr/io.h>
#include "rtc.h"
#include "sendbyte.h" 

#pragma pack(1)

typedef struct {
    uint8_t green;
    uint8_t red;
    uint8_t blue;
} color;

void set_binary_display(time);

#endif