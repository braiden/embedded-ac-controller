#pragma once

#include <avr/pgmspace.h>
#include <inttypes.h>

#define LISTEN_SOCK_MASK 0x10
#define E_SOCK_MASK 0x20

#define SOCK_ERR_UNAVAILIBLE 0x21
#define SOCK_ERR_TIMEOUT 0x22
#define SOCK_ERR_CLOSED 0x23

#define FD_SET(fdset, fd) fdset |= fd & LISTEN_SOCK_MASK ? fd << 4 : _BV(fd)
#define FD_CLEAR(fdset, fd) fdset &= ~(fd & LISTEN_SOCK_MASK ? fd << 4 : _BV(fd))
#define FD_ZERO(fdset) fdset = 0x00
#define FD_ISSET(fdset, fd) (fd & LISTEN_SOCK_MASK ? fdset & (fd << 4) : fdset & _BV(fd))

#define SOCK_ERR(fd) (fd & E_SOCK_MASK)

#ifndef SOCK_NO_SERVER
uint8_t sock_listen(uint16_t port, uint8_t nconcurrent);
uint8_t sock_accept(uint8_t listen_fd);
#endif

#ifndef SOCK_NO_CLIENT
uint8_t sock_connect(uint8_t *addr, uint16_t port);
#endif

uint8_t sock_write(uint8_t sockfd, char *data, uint8_t len);
uint8_t sock_write_P(uint8_t sockfd, PGM_P data, uint8_t len);
uint8_t sock_read(uint8_t sockfd, char *buffer, uint8_t len);
void sock_close(uint8_t sockfd);
uint8_t sock_select(uint8_t readsockfds, uint8_t timeout);
