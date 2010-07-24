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

#include <avr/pgmspace.h>

#define ROMFS_DIRECTORY 0

#define ROMFS_IS_DIRECTORY(node) ((node).file_attr & _BV(ROMFS_DIRECTORY))

typedef struct romfs_node {
	uint8_t filename_len;
	uint8_t file_attr;
	uint16_t data_len;
	PGM_P filename;
	PGM_VOID_P data;
} romfs_node_t;

// the linker must export this symbol
// which points to binary blob in .text.
// the address is in program space, and
// not directly readable.
extern romfs_node_t romfs_text;

void romfs_root(romfs_node_t *rootnode);
uint8_t romfs_open(romfs_node_t *dirnode, romfs_node_t *result, char *fname);
