#ifndef _RTC_H
#define _RTC_H

#include <avr/io.h>

#define RTC_DEVICE_ADDRESS 0x68                               
#define TW_WRITE 0x00
#define TW_READ 0x01

#define RTC_TIME_OFFSET 0x00            // 0x00 seconds, 0x01 minutes, 0x02 hours
#define RTC_CONFIG_OFFSET 0x07
#define RTC_STATUS_OFFSET 0x0f

#pragma pack(1)

/* Struct storing time */
typedef struct {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
} time;

/* Struct storing date */
typedef struct {
    uint8_t day;
    uint8_t month;
    uint8_t year;
} date;

/* Initialize rtc */
void rtc_init(void);

/* Read time from rtc */
time rtc_get_time(void);

/* Set time of rtc */
void rtc_set_time(time, date);

/* Check if second is updated */
uint8_t rtc_is_second(void);

#endif