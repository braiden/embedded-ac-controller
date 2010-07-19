#include <avr/io.h>
#include <util/delay.h>
#include "spi.h"

void spi_init()
{
	// MISO is input
	SPI_DDR &= ~_BV(SPI_MISO);
	// MOSI and SCK are out
	SPI_DDR |= _BV(SPI_MOSI) | _BV(SPI_SCK);
}

void spi_write_byte(uint8_t data)
{
	uint8_t n=0;
	for(; n < 8; n++) {
		// set output state for bit
		if (data & 0x80) {
			SPI_PORT |= _BV(SPI_MOSI);
		} else {
			SPI_PORT &= ~_BV(SPI_MOSI);
		}
		// strobe clock
		SPI_PIN |= _BV(SPI_SCK);
		asm("nop");
		SPI_PIN |= _BV(SPI_SCK);
		data <<= 1;
	}
}

uint8_t spi_read_byte() 
{
	uint8_t result = 0x00;
	uint8_t n = 0;
	for(; n < 8; n++) {
		result <<= 1;
		// raise clock
		SPI_PIN |= _BV(SPI_SCK);
		asm("nop");
		// read value
		if (SPI_PIN & _BV(SPI_MISO)) {
			result |= 0x01;
		}
		// clock low
		SPI_PIN |= _BV(SPI_SCK);
	}
	return result;
}
