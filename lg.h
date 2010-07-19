#pragma once

// IR carrier frequency
#define LG_CARRIER 38000L
// Number of units of time (1/LG_CARRIER) to transmit one bit
#define LG_BIT_LENGTH 43
// Number of units of time (1/LG_CARRIER) which output is active/on
#define LG_BIT_ACTIVE 22
// Number of bit lengths (LG_BIT_LENGTH) for which to send prolog
#define LG_PROLOG_LENGTH 12
// Numoer of bit lengths (LG_BIT_LENGTH) for which prolog output is active/on
#define LG_PROLOG_ACTIVE 8

#define LG_CMD_POWER 1
#define LG_CMD_TEMP_UP 3
#define LG_CMD_TEMP_DOWN 2
#define LG_CMD_FAN_SPEED 4
#define LG_CMD_TIMER 5
#define LG_CMD_MODE 6
#define LG_CMD_ENERGY_SAVE 7
#define LG_CMD_POWER_OFF 0

#define LG_FAN_HIGH 3
#define LG_FAN_MED 2
#define LG_FAN_LOW 1

void lg_init();
void lg_send(uint8_t *opcode);
void lg_cmd(uint8_t cmd);
uint8_t lg_checksum(uint8_t *buffer, uint8_t len);
void lg_create_cmd(uint8_t temp, uint8_t fan, uint8_t *buffer);
