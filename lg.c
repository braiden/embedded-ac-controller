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

#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stddef.h>
#include "lg.h"

uint8_t commands[] PROGMEM = {
	0x1C,0x88,0xC0,0x05,0x10, // power off
	0x20,0x81,0x66,0x81,0x7E, // power toggle
	0x20,0x81,0x66,0x51,0xAE, // temp up
	0x20,0x81,0x66,0xA1,0x5E, // temp down
	0x20,0x81,0x66,0x99,0x66, // fan speed toggle
	0x20,0x81,0x66,0xF9,0x06, // timer
	0x20,0x81,0x66,0xD9,0x26, // mode toggle
	0x20,0x81,0x66,0x41,0xBE, // energy save toggle
};

// next code for transmission
volatile uint8_t *next_command = NULL;

// initialize the timer & PWM hardware
void lg_init()
{
	// fast PWM output on OC0B, OCRA defines TOP, fcpu/8 prescalar
	TCCR0A |= _BV(COM0B1) | _BV(COM0B0) | _BV(WGM01) | _BV(WGM00);
	TCCR0B |= _BV(WGM02) | _BV(CS01);
	// set the PWM frequency to 38khz
	OCR0A = ((double)F_CPU) / ((double)LG_CARRIER) / 8.0 - 1; // fudge-factor
	// set the duty cycle to 50%
	OCR0B = OCR0A / 2;
	// interrupt on overflow (pwm cycle) 
	TIMSK0 |= _BV(TOIE0);
}

// send command by name (id)
void lg_cmd(uint8_t cmd)
{
	uint8_t buffer[5];
	memcpy_P(buffer, commands+cmd*5, 5);
	lg_send(buffer);
}

// send a raw opcode
void lg_send(uint8_t *opcode)
{
	// buffer must be static, so value is not destroyed
	// when this method terminates, its needed by ISR
	static uint8_t buffer[5];
	memcpy(buffer, opcode, 5);
	next_command = buffer;
}

// the last nibble (4bit) of ir transmission
// is sum of all data transmitted
uint8_t lg_checksum(uint8_t *buffer, uint8_t len)
{
	uint8_t checksum = 0;
	for (; len; len--) {
		checksum += buffer[len - 1];
	}
	
	return (checksum + (checksum >> 4)) & 0x0F;
}

// create an ir code to set temperature and fan to
// values specified
void lg_create_cmd(uint8_t temp, uint8_t fan, uint8_t *buffer)
{
	switch (fan) {
		case LG_FAN_LOW: fan = 0x00; break;
		case LG_FAN_MED: fan = 0x02; break;
		case LG_FAN_HIGH: fan = 0x04; break;
		default: fan = 0x00;
	}

	if (temp < 60) {
		temp = 0x01;
	} else if (temp >= 60 && temp < 80) {
		temp = (temp - 60) / 2 + 1;
	} else if (temp >= 80 && temp < 86) {
		temp = (temp - 60) / 2 + 2;
	} else {
		temp = 0xFF;
	}

	buffer[0] = 0x1C; // length
	buffer[1] = 0x88; // unit id?
	buffer[2] = 0x00; // power on + mode = cool
	buffer[3] = (temp << 4) | fan;
	buffer[4] = lg_checksum(buffer + 1, 3) << 4;
}

// ISR for comparator match (executed twice per PWM cycle)
ISR(TIMER0_OVF_vect)
{
	static uint8_t tickcount = 0;
	static uint8_t blockcount = 0;
	static uint8_t *command = NULL;
	static uint8_t command_len = 0;
	static uint8_t sent = 0;
	static uint8_t bit = 0;

	// check if a new command is ready
	if (command == NULL && next_command != NULL) {
		command_len = *next_command;
		command = (void *) next_command + 1;
		next_command = NULL;
		tickcount = 0;
		blockcount = 0;
		bit = 0;
		sent = 0;
	}

	if (command != NULL) {
		if (blockcount < LG_PROLOG_ACTIVE) {
			// PWM output is on during initial prolog
			DDRD |= _BV(PIN5);
		} else if (blockcount < LG_PROLOG_LENGTH) {
			// PWM output is off for second part of prolog
			DDRD &= ~_BV(PIN5);
		} else if (sent <= command_len && tickcount == 0) {
			if (sent == 0) {
				// the signal always starts high
				// data is transmitted in silence between pulses
				DDRD |= _BV(PIN5);
				sent ++;
			} else if (
					(bit & 0xF0) || // we're tranmitting the second half of a one 
					!((0x80>>(bit & 0x0F)) & (*command)) ) { // the current bit is 0
				// output is on
				DDRD |= _BV(PIN5);
				// clear the zero flag, and increment bit
				bit &= 0x0F;
				bit++;
				sent++;
				if (bit > 7) {
					// bit overflowed, move to next byte
					command++;
					bit = 0;
				}
			} else {
				// at a one, we don't increment the bit counter,
				// just pause to one is a longer period
				DDRD &= ~_BV(PIN5);
				bit |= 0xF0;
			}
		} else if (tickcount == LG_BIT_ACTIVE) {
			// pause between bits
			DDRD &= ~_BV(PIN5);
		} else if (tickcount == 0) {
			// done with this command
			DDRD &= ~_BV(PIN5);
			command = NULL;
		}

		// increment our state
		tickcount++;
		if (tickcount >= LG_BIT_LENGTH) {
			tickcount = 0;
			blockcount++;
		}
	}
}
