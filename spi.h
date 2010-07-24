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
