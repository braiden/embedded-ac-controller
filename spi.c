#include <avr/io.h>
#include <util/delay.h>
#include "spi.h"

void spi_init()
{
	// MISO is input
	SPI_DDR &= ~_BV(SPI_MISO);
	// MOSI and SCK are out
	SPI_DDR |= _BV(SPI_MOSI) | _BV(SPI_SCK);
	// Enable SPI Master, MSB, SCK = F_CPU / 2
	SPCR |= _BV(SPE) | _BV(MSTR);
	SPSR |= _BV(SPI2X);
}

uint8_t spi_write_byte(uint8_t data)
{
	SPDR = data;
	loop_until_bit_is_set(SPSR, SPIF);
	return SPDR;
}