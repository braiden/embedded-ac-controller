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

#include <avr/interrupt.h>
#include <avr/io.h>
#include "ratelimit.h"
#include "debug.h"

#define STRINGIFY2(i) #i
#define STRINGIFY(i) STRINGIFY2(i)

volatile uint8_t count = 0;

// initialize the timer & PWM hardware
void ratelimit_init()
{
	// fast PWM with ICR1 as TOP, clock source = fcpu / 1024
	TCCR1A |= _BV(WGM11);
	TCCR1B |= _BV(WGM13) | _BV(WGM12) |  _BV(CS12) | _BV(CS10);
	// interrupt on timer overflow
	TIMSK1 |= _BV(TOIE1);
	// set the timer overflow to 1 per TIMER_OVF_SECONDS seconds
	ICR1H = (uint8_t)((F_CPU / 1024 * TIMER_OVF_SECONDS) >> 8);
	ICR1L = (uint8_t)(F_CPU / 1024 * TIMER_OVF_SECONDS);
}

uint8_t ratelimit()
{
	if (count >= RATE_LIMIT) {
		return 0;
	} else {
		count++;
		return 1;
	}
}

// timer overflow
ISR(TIMER1_OVF_vect)
{
	static uint8_t seconds = 0;
	
	if (seconds % 10 == 0) {
		log("TIMER_OVF_vect(): tick=");
		log_int(seconds, 1, 10);
		log("/" STRINGIFY(RATE_LIMIT_PRESCALAR) "\n");
	}

	if (seconds >= RATE_LIMIT_PRESCALAR) {
		seconds = 0;
		count = 0;
		log("TIMER1_OVF_vect(): Reset Rate Counter.\n");
	} else {
		seconds++;
	}
}
