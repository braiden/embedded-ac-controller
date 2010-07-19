#pragma once

#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>
#include "uart.h"

#ifdef DEBUG
// log a literal string
#define log(msg) uart_send_P(PSTR(msg))
// log the string pointed to by str
#define log_str(str) uart_send(str)
// log the string in progmem pointed to by pstr
#define log_str_P(pstr) uart_send_P(pstr)
// log char
#define log_char(c) uart_send_char(c)

#define log_uint8(n) log_int(n, 2, 16)

#define log_uint16(n) log_int(n, 4, 16)

#define log_int(n, len, base) {\
	uint8_t c_;\
	char buffer_[5];\
	itoa(n, buffer_, base);\
	for (c_ = strlen(buffer_); c_ < len; c_++) {\
		uart_send_char('0');\
	}\
	uart_send(buffer_);\
}

#else

#define log(msg)
#define log_str(str)
#define log_str_P(pstr)
#define log_char(c)
#define log_uint8(n)
#define log_uint16(n)
#define log_int(n, len, base)

#endif
