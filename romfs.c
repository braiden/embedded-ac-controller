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
#include <avr/pgmspace.h>
#include "romfs.h"
#include "uart.h"
#include "debug.h"

void romfs_root(romfs_node_t *node)
{
	memcpy_P(node, &romfs_text, sizeof(romfs_node_t));
	node->filename = (PGM_P)&romfs_text + 4;
	node->data = (PGM_VOID_P)&romfs_text + 4 + node->filename_len;
}

uint8_t romfs_open(romfs_node_t *dirnode, romfs_node_t *result, char *fname)
{
	if (ROMFS_IS_DIRECTORY(*dirnode)) {
		PGM_VOID_P ptr = dirnode->data;

		log("dir: ");
		log_str_P(dirnode->filename);
		log("\n");

		while (ptr < dirnode->data + dirnode->data_len) {
			log_str_P((PGM_P)ptr + 4);
			log_str_P(PSTR("\n"));
			memcpy_P(result, ptr, sizeof(romfs_node_t));

			if (strcmp_P(fname, (PGM_P)ptr + 4) == 0) {
				log("Found\n");
				// we found a filename match, finish init
				result->filename = (PGM_P)ptr + 4;
				result->data = ptr + 4 + result->filename_len;
				return 1;
			}
			ptr += 4 + result->filename_len + result->data_len;	
		}
	} else {
		log("E: non-dir\n");
	}

	return 0;
}
