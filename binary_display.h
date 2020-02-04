#ifndef _BINARY_DISPLAY_H
#define _BINARY_DISPLAY_H

#include <avr/io.h>
#include "rtc.h"
#include "sendbyte.h" 

#pragma pack(1)

struct colour {
    uint8_t green;
    uint8_t red;
    uint8_t blue;
};

void set_binary_display(struct time);

#endif