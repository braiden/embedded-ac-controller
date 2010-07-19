#pragma once

#include <avr/io.h>

#ifndef BAUD 
#define BAUD 9600
#endif

void uart_init();
void uart_send(char* data);
void uart_send_P(PGM_P data);
void uart_send_char(char c);
