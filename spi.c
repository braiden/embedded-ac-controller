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

#include <avr/io.h>
#include <util/delay.h>
#include "spi.h"

void spi_init()
{
	// MISO is input
	SPI_DDR &= ~_BV(SPI_MISO);
	// MOSI and SCK are out
	SPI_DDR |= _BV(SPI_MOSI) | _BV(SPI_SCK);
	// Enable SPI Master, MSB, lowest clock speed (F_CPU / 128)
	// should be 100 - 400khz for initial mmc initialization
	SPCR |= _BV(SPE) | _BV(MSTR) | _BV(SPR1) | _BV(SPR0);
}

void spi_fullspeed()
{
	// SPI clock to F_CPU / 2
	SPSR |= _BV(SPI2X);
	SPCR &= ~(_BV(SPR1) | _BV(SPR0));
	SPCR |= _BV(SPE) | _BV(MSTR);
}

uint8_t spi_write_byte(uint8_t data)
{
	SPDR = data;
	loop_until_bit_is_set(SPSR, SPIF);
	return SPDR;
}
