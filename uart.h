#ifndef _UART_H
#define _UART_H

#define F_CPU 16000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#include "rtc.h"

FILE* uart_init(unsigned long);

int uart_putchar(char, FILE*);

struct time* update_time(void);

#endif