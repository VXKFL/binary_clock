#ifndef _UART_H
#define _UART_H

#ifndef F_CPU
#warning F_CPU not defined.
#define F_CPU 16000000
#endif

#include <avr/io.h>
#include <stdio.h>
#include "rtc.h"

FILE* uart_init(const uint32_t);

int uart_putchar(char, FILE*);

struct time* update_time(void);

#endif