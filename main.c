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

#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "w5100.h"
#include "uart.h"
#include "httpd.h"
#include "lg.h"
#include "socket.h"
#include "mcp9800.h"
#include "debug.h"
#include "diskio.h"
#include "spi.h"
#include "pff.h"
#include "ratelimit.h"

#define RATE_LIMIT_DEFAULT 20
#define RATE_LIMIT_POWER 3

#define TEMP 1
#define FAN 2
#define ERR_RATE_TOO_FAST 2

w5100_config_t W5100_CONFIG PROGMEM = {
	{192,168,0,1},
	{255,255,255,0},
	{'b','r','a','i','d','n'},
	{192,168,0,5}
};

#ifdef SUPPORT_RAW_CMD

uint8_t atoi16(char *str)
{
	uint8_t result = 0;

	for(; *str; str++) {
		result <<= 4;
		if (*str >= '0' && *str <= '9') {
			result += *str - '0';
		} else if (*str >= 'a' && *str <= 'f') {
			result += *str - 'a' + 10;
		} else if (*str >= 'A' && *str <= 'F') {
			result += *str - 'A' + 10;
		}
	}

	return result;
}

uint8_t do_raw_cmd(uint8_t sock, uint8_t status, char *buffer, uint8_t add_len_and_sum)
{
	if (status == HTTPD_READ_QUERY_STRING) {
		status = httpd_read_path(sock, buffer);
	}

	uint8_t len = strlen(buffer);
	
	if (status != HTTPD_READ_DONE) {
		return 0;
	} else if (!add_len_and_sum && (len < 9 || len > 10)) {
		return 0;
	} else if (add_len_and_sum && len != 6) {
		return 0;
	}

	// use part of the receive buffer as our opcode array
	// were assuming buffer is big enough, no bounds checking
	uint8_t *opcode = ((uint8_t*)buffer) + len;

	// convert char arrary into binary opcode
	uint8_t n;
	for (n = (len + 1) / 2 - 1; n != 0xFF ; n--) {
		uint8_t is_one_byte = buffer[n * 2 + 1] == 0;
		buffer[n * 2 + 2] = 0;
		opcode[n] = atoi16(&(buffer[n * 2]));
		if (is_one_byte) {
			opcode[n] <<= 4;
		}
	}
	
	if (add_len_and_sum) {
		// generate the checksum nibble
		if (len % 2) {
			opcode[len / 2] |= lg_checksum(opcode, (len + 1) / 2);
		} else {
			opcode[len / 2] = lg_checksum(opcode, (len +1) / 2) << 4;
		}
		// add the bit length to start of opcode
		*--opcode = len * 4 + 4;
	}

#ifdef DEBUG
	log("cmd=");
	for (n = 0; n < 5; n++) {
		log_uint8(opcode[n]);
	}
	log("\n");
#endif

	// send the op code
	if (ratelimit(RATE_LIMIT_DEFAULT)) {
		lg_send(opcode);
		return 1;
	} else {
		return ERR_RATE_TOO_FAST;
	}
}

#endif

uint8_t do_on_cmd(uint8_t sock, uint8_t status, char *buffer)
{
	uint8_t temp = 70;
	uint8_t fan = 1;

	if (status == HTTPD_READ_QUERY_STRING) {
		do {
			uint8_t param = 0;
			uint8_t value;
			status = httpd_read_path(sock, buffer);
			if (status == HTTPD_READ_QUERY_VALUE) {
				// the param name is in buffer, next read will
				// given the value.
				if (strcmp_P(buffer, PSTR("temp")) == 0) {
					param = TEMP;
				} else if (strcmp_P(buffer, PSTR("fan")) == 0) {
					param = FAN;
				}
				status = httpd_read_path(sock, buffer);
				value = atoi(buffer);
				if (param == FAN) {
					fan = value;
				} else if (param == TEMP) {
					temp = value;
				}
			}
		} while (status);
	} else if (status != HTTPD_READ_DONE) {
		return 0; // 404, "on" was refrences as a directory
	}

	log("fan=");
	log_uint8(fan);
	log(", temp=");
	log_uint8(temp);
	log("\n");

	uint8_t *cmd = (uint8_t*) buffer;
	lg_create_cmd(temp, fan, cmd);
	
#ifdef DEBUG
	log("cmd=");
	for (fan = 0; fan < 5; fan++) {
		log_uint8(cmd[fan]);
	}
	log("\n");
#endif
	
	if (ratelimit(RATE_LIMIT_DEFAULT)) {
		lg_send(cmd);
		return 1;
	} else {
		return ERR_RATE_TOO_FAST;
	}
}

uint8_t ratelimit_lg_cmd(uint8_t cmd, uint8_t limit)
{
	if (ratelimit(limit)) {
		lg_cmd(cmd);
		return 0;
	} else {
		return ERR_RATE_TOO_FAST;
	}
}

void cgi_handler(uint8_t sock, uint8_t method, char *buffer)
{
	uint8_t status = httpd_read_path(sock, buffer);
	if (status == HTTPD_READ_PATH && strcmp_P(buffer, PSTR("ac")) == 0)
	{
		// found a directory named 'ac', read the command
		status = httpd_read_path(sock, buffer);
		if (status == HTTPD_READ_DONE || status == HTTPD_READ_QUERY_STRING) {
			// found a 'file', map to command
			if (strcmp_P(buffer, PSTR("power")) == 0) {
				status = ratelimit_lg_cmd(LG_CMD_POWER, RATE_LIMIT_POWER);
			} else if (strcmp_P(buffer, PSTR("temp-down")) == 0) {
				status = ratelimit_lg_cmd(LG_CMD_TEMP_DOWN, RATE_LIMIT_DEFAULT);
			} else if (strcmp_P(buffer, PSTR("temp-up")) == 0) {
				status = ratelimit_lg_cmd(LG_CMD_TEMP_UP, RATE_LIMIT_DEFAULT);
			} else if (strcmp_P(buffer, PSTR("energy")) == 0) {
				status = ratelimit_lg_cmd(LG_CMD_ENERGY_SAVE, RATE_LIMIT_DEFAULT);
			} else if (strcmp_P(buffer, PSTR("mode")) == 0) {
				status = ratelimit_lg_cmd(LG_CMD_MODE, RATE_LIMIT_DEFAULT);
			} else if (strcmp_P(buffer, PSTR("timer")) == 0) {
				status = ratelimit_lg_cmd(LG_CMD_TIMER, RATE_LIMIT_DEFAULT);
			} else if (strcmp_P(buffer, PSTR("speed")) == 0) {
				status = ratelimit_lg_cmd(LG_CMD_FAN_SPEED, RATE_LIMIT_DEFAULT);
			} else if (strcmp_P(buffer, PSTR("off")) == 0) {
				status = ratelimit_lg_cmd(LG_CMD_POWER_OFF, RATE_LIMIT_DEFAULT);
			} else if (strcmp_P(buffer, PSTR("on")) == 0) {
				// parse the 'on' command
				if((status = do_on_cmd(sock, status, buffer)) == 0) {
					goto NOTFOUND;
				}
#ifdef SUPPORT_RAW_CMD
			} else if (strcmp_P(buffer, PSTR("cmd")) == 0) {
				if (!do_raw_cmd(sock, status, buffer, 1)) {
					goto ERR;
				}
			} else if (strcmp_P(buffer, PSTR("raw")) == 0) {
				if (!do_raw_cmd(sock, status, buffer, 0)) {
					goto ERR;
				}
#endif
			} else {
				goto NOTFOUND;
			}
			
			if (status != ERR_RATE_TOO_FAST) {
				goto OK;
			} else {
				goto ERR;
			}
		}
#ifdef SUPPORT_MCP9800
	} else if (status == HTTPD_READ_PATH && strcmp_P(buffer, PSTR("temp")) == 0) {
		status = httpd_read_path(sock, buffer);
		if (status == HTTPD_READ_DONE || status == HTTPD_READ_QUERY_STRING) {
			if (strcmp_P(buffer, PSTR("raw")) == 0) {
				uint8_t temp;
				httpd_write_response(sock, HTTPD_OK);
				sock_write_P(sock, PSTR("Content-Type: text/plain\r\n\r\n"), 28);
				temp = mcp9800_get_temp();
				itoa(temp, buffer, 16);
				sock_write( sock, buffer, strlen(buffer));
				return;
			}

		}
#endif
	}

NOTFOUND:
	httpd_write_response(sock, HTTPD_NOTFOUND);
	return;

ERR:
	httpd_write_response(sock, HTTPD_ERR);
	return;

OK:
	httpd_write_response(sock, HTTPD_OK);
	sock_write_P(sock, PSTR("Content-Type: text/plain\r\n\r\nOK"), 30);	
	return;
}

int main()
{
	FATFS *fs = NULL;

	wdt_enable(WDTO_8S);
	_delay_ms(2);

	uart_init();
	lg_init();
	spi_init();
	w5100_init(&W5100_CONFIG);
	ratelimit_init();

	//enable cd card CS
	DDRD |= _BV(PIN3);
	PORTD |= _BV(PIN3);
#ifdef SUPPORT_MMC
	FATFS tmp;
	// try to init micro SD
	if (disk_initialize() != STA_NOINIT) {
		// once initial inti is done enable full speed SPI
		if(pf_mount(&tmp) == FR_OK) {
			fs = &tmp;
		} else {
			log("pf_mount FAILED\n");
		}
	} else {
		log("disk_init FAILED\n");
	}
#endif
	spi_fullspeed();

	httpd_init();
#ifdef SUPPORT_MCP9800
	mcp9800_init();
#endif
	sei();

#ifdef DEBUG
	uint8_t n;
	log("cfg=");
	for (n=0; n<6 ; n++) {
		log_uint8(w5100_mem_read(W5100_COMMON_REG, W5100_SHAR0+n));
		if (n<5) log_char(':');
	}
	log_char(' ');

	for (n=0; n<4; n++) {
		log_int(w5100_mem_read(W5100_COMMON_REG, W5100_SIPR0+n),1,10);
		if (n<3) log_char('.');
	}
	log_char('/');

	for (n=0; n<4; n++) {
		log_int(w5100_mem_read(W5100_COMMON_REG, W5100_SUBR0+n),1,10);
		if (n<3) log_char('.');
	}
	log_char(' ');

	for (n=0; n<4; n++) {
		log_int(w5100_mem_read(W5100_COMMON_REG, W5100_GAR0+n),1,10);
		if (n<3) log_char('.');
	}
	log("\n");
#endif

	uint8_t loop = 0;
	while (1) {
		wdt_reset();
		httpd_loop(cgi_handler);
		if (++loop == 0) {
			sock_dump_netstat();
		}
	}

	return 0;
}
