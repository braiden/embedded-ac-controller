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
