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
#include <avr/pgmspace.h>
#include "spi.h"

#ifndef W5100_PORT
#define W5100_PORT PORTB
#endif
#define W5100_DDR (*(&SPI_PORT-1))
#define W5100_PIN (*(&SPI_PORT-2))

#ifndef W5100_CS
#define W5100_CS PIN2
#endif

#ifndef W5100_RST
#define W5100_RST PIN1
#endif

/* SPI COMMANDS */
#define W5100_CMD_READ 0x0F
#define W5100_CMD_WRITE 0xF0

/* MEMORY MAP */
#define W5100_COMMON_REG 0x00
#define W5100_SOCKET_REG 0x04
#define W5100_TX_MEMORY 0x40
#define W5100_RX_MEMORY 0x60

/* COMMON REGISTERS */
#define W5100_MR 0x00
#define W5100_GAR0 0x01
#define W5100_GAR1 0x02
#define W5100_GAR2 0x03
#define W5100_GAR3 0x04
#define W5100_SUBR0 0x05
#define W5100_SUBR1 0x06
#define W5100_SUBR2 0x07
#define W5100_SUBR3 0x08
#define W5100_SHAR0 0x09
#define W5100_SHAR1 0x0A
#define W5100_SHAR2 0x0B
#define W5100_SHAR3 0x0C
#define W5100_SHAR4 0x0D
#define W5100_SHAR5 0x0E
#define W5100_SIPR0 0x0F
#define W5100_SIPR1 0x10
#define W5100_SIPR2 0x11
#define W5100_SIPR3 0x12
#define W5100_IR 0x15
#define W5100_IMR 0x16
#define W5100_RTR0 0x17
#define W5100_RTR1 0x18
#define W5100_RCR 0x19
#define W5100_RMSR 0x1A
#define W5100_TMSR 0x1B
#define W5100_PATR0 0x1C
#define W5100_PATR1 0x1D
#define W5100_PTIMER 0x28
#define W5100_PMAGIC 0x29
#define W5100_UIPR0 0x2A
#define W5100_UIPR1 0x2B
#define W5100_UIPR2 0x2C
#define W5100_UIPR3 0x2D
#define W5100_UPORT0 0x2E
#define W5100_UPORT1 0x2F

/* SOCKET REGISTERS */
#define W5100_Sn_MR 0x00
#define W5100_Sn_CR 0x01
#define W5100_Sn_IR 0x02
#define W5100_Sn_SR 0x03
#define W5100_Sn_PORT0 0x04
#define W5100_Sn_PORT1 0x05
#define W5100_Sn_DHAR0 0x06
#define W5100_Sn_DHAR1 0x07
#define W5100_Sn_DHAR2 0x08
#define W5100_Sn_DHAR3 0x09
#define W5100_Sn_DHAR4 0x0A
#define W5100_Sn_DHAR5 0x0B
#define W5100_Sn_DIPR0 0x0C
#define W5100_Sn_DIPR1 0x0D
#define W5100_Sn_DIPR2 0x0E
#define W5100_Sn_DIPR3 0x0F
#define W5100_Sn_DPORT0 0x10
#define W5100_Sn_DPORT1 0x11
#define W5100_Sn_MSSR0 0x12
#define W5100_Sn_MSSR1 0x13
#define W5100_Sn_PROTO 0x14
#define W5100_Sn_TOS 0x15
#define W5100_Sn_TTL 0x16
#define W5100_Sn_TX_FSR0 0x20
#define W5100_Sn_TX_FSR1 0x21
#define W5100_Sn_TX_RD0 0x22
#define W5100_Sn_TX_RD1 0x23
#define W5100_Sn_TX_WR0 0x24
#define W5100_Sn_TX_WR1 0x25
#define W5100_Sn_RX_RSR0 0x26
#define W5100_Sn_RX_RSR1 0x27
#define W5100_Sn_RX_RD0 0x28
#define W5100_Sn_RX_RD1 0x29

/* SOCKET COMMANDS */
#define W5100_OPEN 0x01
#define W5100_LISTEN 0x02
#define W5100_CONNECT 0x04
#define W5100_DISCON 0x08
#define W5100_CLOSE 0x10
#define W5100_SEND 0x20
#define W5100_SEND_MAC 0x21
#define W5100_SEND_KEEP 0x22
#define W5100_RECV 0x40

/* SOCKET PROTOCOL */
#define W5100_CLOSED 0x00
#define W5100_TCP 0x01
#define W5100_UDP 0x02
#define W5100_IPRAW 0x03

/* INTERRUPT BITS */
#define W5100_IR_CONFLICT 7
#define W5100_IR_UNREACH 6
#define W5100_IR_S3_INT 3
#define W5100_IR_S2_INT 2
#define W5100_IR_S1_INT 1
#define W5100_IR_S0_INT 0

/* SOCK INT BITS */
#define W5100_Sn_IR_SEND_OK 4
#define W5100_Sn_IR_TIMEOUT 3
#define W5100_Sn_IR_RECV 2
#define W5100_Sn_IR_DISCON 1
#define W5100_Sn_IR_CON 0

/* STATUS */
#define W5100_SOCK_CLOSED 0x00
#define W5100_SOCK_LISTEN 0x14
#define W5100_SOCK_ESTABLISHED 0x17
#define W5100_SOCK_CLOSE_WAIT 0x1C
#define W5100_SOCK_UDP 0x22

/* SOCK_RW constants */
#define W5100_READ 0
#define W5100_WRITE 1
#define W5100_WRITE_P 2

typedef struct w5100_config {
	uint8_t gateway[4];
	uint8_t netmask[4];
	uint8_t hwaddr[6];
	uint8_t ipaddr[4];
} w5100_config_t;

void w5100_init(PGM_VOID_P w5100_cfg);
void w5100_mem_write(uint8_t haddr, uint8_t laddr, uint8_t value);
void w5100_mem_write_P(uint8_t haddr, uint8_t laddr, PGM_VOID_P values, uint8_t len);
uint8_t w5100_mem_read(uint8_t haddr, uint8_t laddr);
uint8_t w5100_sock_rw(uint8_t socket, void *data, uint8_t len, uint8_t direction);
void w5100_drain(uint8_t socket);
