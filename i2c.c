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

#include <stdint.h>
#include <avr/io.h>
#include <util/twi.h>
#include "i2c.h"
#include "debug.h"

#ifdef SUPPORT_MCP9800

void i2c_init()
{
	// Use interal pull-up resisters
	PORTC |= _BV(PIN5) | _BV(PIN4);
	// prescalar 16
	TWSR |= _BV(TWPS1); 
	TWSR &= ~_BV(TWPS0);
	// set bit rate
	TWBR = ((F_CPU / I2C_CLK) - 16UL) / 32UL;
}

uint8_t i2c_start()
{
	// send the start bit
	TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
	log("START ");
	// wait for transmission to complete
	loop_until_bit_is_set(TWCR, TWINT);
	log_uint16(TW_STATUS);
	// verify we're in the start state
	if (TW_STATUS != TW_START && TW_STATUS != TW_REP_START) {
		log(" FAILED\n");
		return 1;
	}
	log(" OK\n");
	return 0;
}

void i2c_stop()
{
	log("STOP\n");
	TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);
}

uint8_t i2c_write(uint8_t addr, uint8_t *buffer, uint8_t len)
{
	// transmit the start bit
	if (i2c_start()) {
		return 1;
	}
	// set the write address
	TWDR = addr << 1 | TW_WRITE;
	// transmite the address
	TWCR = _BV(TWINT) | _BV(TWEN);
	log("SLA_ACK ");
	loop_until_bit_is_set(TWCR, TWINT);
	log_uint16(TW_STATUS);
	if (TW_STATUS != TW_MT_SLA_ACK) {
		log(" FAILED\n");
		return 1;
	}
	log(" OK\n");
	// write the data
	while (len--) {
		// next byte to transmit
		TWDR = *buffer++;
		// send it
		TWCR = _BV(TWINT) | _BV(TWEN);
		log("DATA_ACK ");
		loop_until_bit_is_set(TWCR, TWINT);
		log_uint16(TW_STATUS);
		if (TW_STATUS != TW_MT_DATA_ACK) {
			log(" FAILED\n");
			return 1;
		}
		log(" OK\n");
	}
	return 0;
}

uint8_t i2c_read(uint8_t addr, uint8_t *buffer, uint8_t len)
{
	// start bit
	if (i2c_start()) {
		return 1;
	}
	// set read addresss
	TWDR = addr << 1 | TW_READ;
	// send address
	TWCR = _BV(TWINT) | _BV(TWEN);
	log("SLA_ACK ");
	loop_until_bit_is_set(TWCR, TWINT);
	log_uint16(TW_STATUS);
	if (TW_STATUS != TW_MR_SLA_ACK) {
		log(" FAILED\n");
		return 1;
	}
	log(" OK\n");
	// read data
	while(len--) {
		if (len) {
			// ack send ack
			TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWEA);
		} else {
			// last byte is nack
			TWCR = _BV(TWINT) | _BV(TWEN);
		}
		log("READ ");
		loop_until_bit_is_set(TWCR, TWINT);
		log_uint16(TW_STATUS);
		switch (TW_STATUS) {
			case TW_MR_DATA_NACK:
				len = 0;
			case TW_MR_DATA_ACK:
				log(" OK\n");
				*buffer++ = TWDR;
				break;
			default:
				log(" FAILED\n");
		}
	}
	return 0;
}

#endif
