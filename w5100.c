#include <stdlib.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "w5100.h"
#include "spi.h"

void w5100_init(PGM_VOID_P w5100_config)
{
	// bring reset pin low
	W5100_DDR |= _BV(W5100_RST);
	W5100_PORT &= ~_BV(W5100_RST);
	// wait for power to stablize (and probably much longer)
	_delay_ms(10);
	// clear RST
	W5100_PORT |= _BV(W5100_RST);
	// initialize spi interface
    spi_init();
	// CS pin is output
	W5100_DDR |= _BV(W5100_CS);
	W5100_PORT |= _BV(W5100_CS);
	// write interface configuration to w5100 (ip, netmask, gateway, hwaddr)
	w5100_mem_write_P(W5100_COMMON_REG, W5100_GAR0, w5100_config, sizeof(w5100_config_t));
}

void w5100_cs()
{
	W5100_PIN |= _BV(W5100_CS);
}

void w5100_mem_write(uint8_t haddr, uint8_t laddr, uint8_t value)
{
	w5100_cs();
	spi_write_byte(W5100_CMD_WRITE);
	spi_write_byte(haddr);
	spi_write_byte(laddr);
	spi_write_byte(value);
	w5100_cs();
}

uint8_t w5100_mem_read(uint8_t haddr, uint8_t laddr)
{
	uint8_t result;
	w5100_cs();
	spi_write_byte(W5100_CMD_READ);
	spi_write_byte(haddr);
	spi_write_byte(laddr);
	result = spi_read_byte();
	w5100_cs();
	return result;
}

void w5100_mem_write_P(uint8_t haddr, uint8_t laddr, PGM_VOID_P values, uint8_t len)
{
	while (len--) {
		uint8_t byte = pgm_read_byte(values);
		w5100_mem_write(haddr, laddr, byte);
		values++;
		laddr++;
	}
}

uint8_t w5100_sock_rw(uint8_t socket, void *data, uint8_t len, uint8_t write)
{
	// the higher byte of address to this sockets registers
	uint8_t sock_reg = W5100_SOCKET_REG + socket;
	// the higher byte of address offset to this sockets rx/tx buffer
	uint8_t sock_high_addr = (write ? W5100_TX_MEMORY : W5100_RX_MEMORY)  + socket * 0x08;

	// get the availble buffer size
	uint8_t free_size_l = w5100_mem_read(sock_reg, write ? W5100_Sn_TX_FSR1 : W5100_Sn_RX_RSR1);

	if (len > free_size_l) {
		if (!w5100_mem_read(sock_reg, write ? W5100_Sn_TX_FSR0 : W5100_Sn_RX_RSR0)) {
			len = free_size_l;
		}
	}

	uint8_t bytes_read = len;

	if (len) {
		// get the unadjusted address of this sockets read / write pointer	
		uint8_t ptr_h = write ? W5100_Sn_TX_WR0 : W5100_Sn_RX_RD0;
		uint8_t ptr_l = write ? W5100_Sn_TX_WR1 : W5100_Sn_RX_RD1;
		uint8_t addr_h = w5100_mem_read(sock_reg, ptr_h);
		uint8_t addr_l = w5100_mem_read(sock_reg, ptr_l);
		while (len--) {
			// caluclate the read addres of buffer
			uint8_t phys_addr = (addr_h & 0x07) + sock_high_addr;
			if (!write && data != NULL) {
				// read a byte from address
				*(char *)data++ = w5100_mem_read(phys_addr, addr_l);
			} else if (write == 1) {
				// write byte to address
				w5100_mem_write(phys_addr,addr_l,*((char *)data++));
			} else if (write == 2) {
				// write progmem byte to address
				w5100_mem_write(phys_addr,addr_l,pgm_read_byte((PGM_P)data++));
			}
			// increment our address pointer
			addr_l++;
			if (!addr_l) {
				addr_h++;
			}
		}
		// update the buffer cursor and commit
		w5100_mem_write(sock_reg, ptr_h, addr_h);
		w5100_mem_write(sock_reg, ptr_l, addr_l);
		w5100_mem_write(sock_reg, W5100_Sn_CR, write ? W5100_SEND : W5100_RECV);
	}

	return bytes_read;
}

void w5100_drain(uint8_t sock)
{
	// the higher byte of address to this sockets registers
	uint8_t sock_reg = W5100_SOCKET_REG + sock;

	// get the current cursor position
	uint16_t read_ptr = w5100_mem_read(sock_reg, W5100_Sn_RX_RD0) << 8
			| w5100_mem_read(sock_reg, W5100_Sn_RX_RD1);

	// get the size of unread data
	uint16_t read_sz = w5100_mem_read(sock_reg, W5100_Sn_RX_RSR0) << 8
			| w5100_mem_read(sock_reg, W5100_Sn_RX_RSR1);

	// calculate new pointer
	read_ptr += read_sz;

	// update the pointer to mark all data as read
	w5100_mem_write(sock_reg, W5100_Sn_RX_RD0, read_ptr >> 8);
	w5100_mem_write(sock_reg, W5100_Sn_RX_RD1, read_ptr);
	w5100_mem_write(sock_reg, W5100_Sn_CR, W5100_RECV);
}
