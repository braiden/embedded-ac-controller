#pragma once

#define MCP9800_ADDR 0x48

#define MCP9800_REG_TEMP 0x00
#define MCP9800_REG_CONFIG 0x01
#define MCP9800_REG_HYSTERESIS 0x02
#define MCP9800_REG_LIMIT 0x03

void mcp9800_init();
uint8_t mcp9800_get_temp();
