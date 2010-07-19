#include <avr/io.h>
#include <avr/pgmspace.h>
#include "uart.h"

void uart_init() {
#include <util/setbaud.h>
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
#if USE_2X
	UCSR0A |= _BV(U2X0);
#else
	UCSR0A &= ~_BV(U2X0);
#endif
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
}

void uart_send_P(PGM_P str)
{
	char c;
	do {
		c = pgm_read_byte(str++);
		uart_send_char(c);
	} while(c);
}

void uart_send(char *c) {
	for (; *c; uart_send_char(*c++));
}

void uart_send_char(char c) {
	if (c == '\n') uart_send_char('\r');
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
}
