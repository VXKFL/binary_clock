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
    int8_t second;
    int8_t minute;
    int8_t hour;
} time;

/* Initialize rtc */
void rtc_init(void);

/* Read time from rtc */
time rtc_get_time(void);

/* Get dst status */
uint8_t rtc_get_dst(void);

/* Toggle dst */
void rtc_toggle_dst(void);

/* Set time of rtc */
void rtc_set_time(time, uint8_t dst);

/* Check if second is updated */
uint8_t rtc_is_second(void);

#endif
