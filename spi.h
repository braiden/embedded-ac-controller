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

#include <avr/io.h>

#ifndef SPI_PORT
#define SPI_PORT PORTB
#endif
#define SPI_DDR (*(&SPI_PORT-1))
#define SPI_PIN (*(&SPI_PORT-2))

#ifndef SPI_SCK
#define SPI_SCK PIN5
#endif
#ifndef SPI_MISO
#define SPI_MISO PIN4
#endif
#ifndef SPI_MOSI
#define SPI_MOSI PIN3
#endif
#ifndef SPI_PERIOD_US
#define SPI_PERIOD_US 0
#endif

#define spi_read_byte() spi_write_byte(0xFF)

void spi_init();
void spi_fullspeed();
uint8_t spi_write_byte(uint8_t data);
