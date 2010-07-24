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
#include <util/twi.h>
#include <avr/pgmspace.h>
#include "uart.h"
#include "i2c.h"
#include "mcp9800.h"
#include "debug.h"

#ifdef SUPPORT_MCP9800

void mcp9800_init()
{
	uint8_t config[2] = { MCP9800_REG_CONFIG, 0x20 };
	i2c_init();
	
	// set mcp resolution to 10 bit
	i2c_write(MCP9800_ADDR, config, 2);
	i2c_stop();

	// some debug output, remove this
	log_uint8(mcp9800_get_temp());
	log("\n");
}

uint8_t mcp9800_get_temp()
{
	uint8_t result[2] = { MCP9800_REG_TEMP };

	if (i2c_write(MCP9800_ADDR, result, 1)) {
		return 0;
	}

	if (i2c_read(MCP9800_ADDR, result, 2)) {
		return 0;
	}

	i2c_stop();

	log("C=");
	log_uint8(result[0]);
	log_uint8(result[1]);
	log("\n");

	// we compress the result to one byte
	// 6 msb represent the whole number part
	// of temerature, 2 lsb are the fraction
	return result[0] << 2 | result[1] >> 6;
}

#endif
