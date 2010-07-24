/*
 *  Copyright (c) 2009 Braiden Kindt
 *
 *  Permission is hereby granted, free of charge, to any person
 *  obtaining a copy of this software and associated documentation
 *  files (the "Software"), to deal in the Software without
 *  restriction, including without limitation the rights to use,
 *  copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following
 *  conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *  OTHER DEALINGS IN THE SOFTWARE.
 *
 */

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
