#ifndef _RTC_H
#define _RTC_H

#include <avr/io.h>
#include <avr/interrupt.h>

#define RTC_DEVICE_ADDRESS 0x68
#define RTC_MEMORY_START_ADDRESS 0x00                               // 0x00 seconds, 0x01 minutes, 0x02 hours
#define TW_WRITE 0x00
#define TW_READ 0x01

#pragma pack(1)

/* Struct storing time*/
struct time {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
};

/* Read time from rtc */
struct time get_time(void);

/* Set time of rtc */
void set_time(struct time);

#endif