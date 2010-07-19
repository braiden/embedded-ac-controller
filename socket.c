#include "socket.h"
#include "w5100.h"

uint8_t allocated_socks = 0x00;
uint8_t tcp_source_port = 0x00;

#ifndef SOCK_NO_SERVER
void _sock_cleanup_discon(uint8_t listenfd)
{
	uint8_t n;
	for (n = 0; n < 4 ; n++) {
		if (listenfd & _BV(n)) {
			uint8_t sockreg = W5100_SOCKET_REG + n;
			uint8_t sockint = w5100_mem_read(sockreg, W5100_Sn_IR);
			if (sockint & _BV(W5100_Sn_IR_DISCON)) {
				// listening sockets don't auto return to LISTEN,
				// need to force the state transition 
				w5100_mem_write(sockreg, W5100_Sn_IR, _BV(W5100_Sn_IR_DISCON));
				w5100_mem_write(sockreg, W5100_Sn_CR, W5100_LISTEN);
			}
		}
	}	
}
#else
void _sock_cleanup_discon(uint8_t listenfd) {}
#endif

uint8_t _sock_write(uint8_t sockfd, char *data, uint8_t len, uint8_t writetype)
{
	uint8_t sockreg = W5100_SOCKET_REG + sockfd;
	uint8_t bytes_wrote = 0;
	while (w5100_mem_read(sockreg, W5100_Sn_SR) == W5100_SOCK_ESTABLISHED &&
			!(bytes_wrote = w5100_sock_rw(sockfd, data, len, writetype)));
	return bytes_wrote;
}

#ifndef SOCK_NO_SERVER
uint8_t sock_listen(uint16_t port, uint8_t nconcurrent)
{
	uint8_t sockfd = LISTEN_SOCK_MASK;
	uint8_t sockbit = 0x01;
	uint8_t sockreg = W5100_SOCKET_REG;
	uint8_t porth = port >> 8;
	uint8_t portl = port;

	// try to find ncurrent availible hardware sockets
	while ((sockbit & 0x0F) && (nconcurrent)) {
		if (!(allocated_socks & sockbit)) {
			allocated_socks |= sockbit;
			sockfd |= sockbit;
			// set type to TCP
			w5100_mem_write(sockreg, W5100_Sn_MR, W5100_TCP);
			// set the port
			w5100_mem_write(sockreg, W5100_Sn_PORT0, porth);
			w5100_mem_write(sockreg, W5100_Sn_PORT1, portl);
			// let the w5100 know the socket is configured
			w5100_mem_write(sockreg, W5100_Sn_CR, W5100_OPEN);
			// clear any ints
			w5100_mem_write(sockreg, W5100_Sn_IR, 0xff);
			// open the socket for connections
			w5100_mem_write(sockreg, W5100_Sn_CR, W5100_LISTEN);
			nconcurrent--;
		}
		sockbit<<=1;
		sockreg++;
	}

	// sockfd is a phony file descriptor, the 4 LSB bits
	// are one iff that hardware socket is allocated to
	// this pseduo fd, we set LISTEN_SOCK_MASK to 4 MSB
	// so we can recognize this fd as a listening sock pool.
	return sockfd;
}
#endif

#ifndef SOCK_NO_SERVER
uint8_t sock_accept(uint8_t listenfd)
{
	// need to clean-up disconnected clients, 
	// so those hw sockets can be used to connect
	_sock_cleanup_discon(listenfd);
	uint8_t sock = 0;
	// block until we get a connection
	while (1) {
		uint8_t sockfd = sock & 0x07;
		if (listenfd & _BV(sockfd)) {
			// read the status reg
			uint8_t sockreg = W5100_SOCKET_REG + sockfd;
			uint8_t sockint = w5100_mem_read(sockreg, W5100_Sn_IR);
			// a new client as connected
			if (sockint & _BV(W5100_Sn_IR_CON)) {
				// clear the status reg
				w5100_mem_write(sockreg, W5100_Sn_IR, _BV(W5100_Sn_IR_CON));
				break;
			}
		}
		sock++;
	}
	// return the sockfd of newly connected client
	return sock & 0x07;
}
#endif

#ifndef SOCK_NO_CLIENT
uint8_t sock_connect(uint8_t *addr, uint16_t port)
{
	uint8_t sock;
	// try to find an unused hardware socket
	for (sock = 0; sock < 4; sock++) {
		// if the sock, isn't being used, continue
		if (!((0x01 << sock) & (allocated_socks))) {
			// high byte of this sockets address in w5100
			uint8_t sockreg = W5100_SOCKET_REG + sock;
			uint8_t ir = 0x00;
			uint8_t irmask = _BV(W5100_Sn_IR_CON) | _BV(W5100_Sn_IR_TIMEOUT) | _BV(W5100_Sn_IR_DISCON);
			allocated_socks |= 0x01 << sock;
			tcp_source_port++;
			// use tcp
			w5100_mem_write(sockreg, W5100_Sn_MR, W5100_TCP);
			// create a source port, we have a pool of 255 (0xFF00-0xFFFF)
			// pickup the last recently unused.
			// TODO, should really verify that source port is unused0
			w5100_mem_write(sockreg, W5100_Sn_PORT0, 0xFF);
			w5100_mem_write(sockreg, W5100_Sn_PORT1, tcp_source_port);
			// ip destination port
			w5100_mem_write(sockreg, W5100_Sn_DPORT0, port >> 8);
			w5100_mem_write(sockreg, W5100_Sn_DPORT1, port);
			// ip destination address
			w5100_mem_write(sockreg, W5100_Sn_DIPR0, addr[0]);
			w5100_mem_write(sockreg, W5100_Sn_DIPR1, addr[1]);
			w5100_mem_write(sockreg, W5100_Sn_DIPR2, addr[2]);
			w5100_mem_write(sockreg, W5100_Sn_DIPR3, addr[3]);
			// tell the w5100 configuration is set
			w5100_mem_write(sockreg, W5100_Sn_CR, W5100_OPEN);
			// clear status reg
			w5100_mem_write(sockreg, W5100_Sn_IR, 0xff);
			// initiate connection
			w5100_mem_write(sockreg, W5100_Sn_CR, W5100_CONNECT);
			// wait for something to happen (connect or timeout)
			while (!((ir = w5100_mem_read(sockreg, W5100_Sn_IR)) & irmask));
			// clear status
			w5100_mem_write(sockreg, W5100_Sn_IR, irmask);
			if (ir & _BV(W5100_Sn_IR_TIMEOUT)) {
				sock = SOCK_ERR_TIMEOUT;
			} else if ((ir & _BV(W5100_Sn_IR_DISCON)) && !(ir & _BV(W5100_Sn_IR_CON))) {
				sock = SOCK_ERR_CLOSED;
			}
			break;
		}
	}
	// no hardware sock availible (there are only 4 in w5100)
	if (sock == 4) {
		sock = SOCK_ERR_UNAVAILIBLE;
	}
	return sock;
}
#endif

uint8_t sock_write(uint8_t sockfd, char *data, uint8_t len)
{
	return _sock_write(sockfd, data, len, W5100_WRITE);
}

uint8_t sock_write_P(uint8_t sockfd, PGM_P data, uint8_t len)
{
	return _sock_write(sockfd, (void *)data, len, W5100_WRITE_P);
}

uint8_t sock_read(uint8_t sockfd, char *buffer, uint8_t len)
{
	uint8_t sockreg = W5100_SOCKET_REG + sockfd;
	uint8_t bytes_read = 0;
	// block until at least one byte is read, or connection closed
	while (len && !(bytes_read = w5100_sock_rw(sockfd, buffer, len, W5100_READ)) &&
			w5100_mem_read(sockreg, W5100_Sn_SR) == W5100_SOCK_ESTABLISHED);
	return bytes_read;
}

void sock_close(uint8_t sockfd)
{
	if (sockfd & 0xF0) {
		// we're closing a listening pseduo socket
		// need to close its underlying connections
		uint8_t n;
		for (n = 0 ; n < 4; n++) {
			if (sockfd & _BV(n)) {
				sock_close(n);
			}
		}
	} else {
		// close the W5100 sock
		w5100_mem_write(W5100_SOCKET_REG + sockfd, W5100_Sn_CR, W5100_DISCON);
		// drain any unread data
		w5100_drain(sockfd);
		// mark the socket as avalible for use
		allocated_socks &= ~_BV(sockfd);
	}
}

uint8_t sock_select(uint8_t readsockfds, uint8_t timeout)
{
	_sock_cleanup_discon(readsockfds >> 4);

	uint8_t result = 0;
	uint8_t n;

	// wait for at least one sock to have event
	while (!result) {
		for (n = 0; n < 8; n++) {
			// the 4 LSB represent real hardware sockets
			if (readsockfds & _BV(n) & 0x0F) {
				uint8_t sockreg = W5100_SOCKET_REG + n;
				// event of interest if discon or at least one byte readable
				if (w5100_mem_read(sockreg, W5100_Sn_RX_RSR1) > 0 ||
						w5100_mem_read(sockreg, W5100_Sn_RX_RSR0) > 0 ||
						w5100_mem_read(sockreg, W5100_Sn_IR) & _BV(W5100_Sn_IR_DISCON)) {
					result |= _BV(n);
				}
			// the 4 MSB are pseduo socks were waiting for connect
			} else if (readsockfds & _BV(n)) {
				uint8_t sockreg = W5100_SOCKET_REG + n - 4;
				// has someone connected?
				if (w5100_mem_read(sockreg, W5100_Sn_IR) & _BV(W5100_Sn_IR_CON)) {
					result |= _BV(n);
				}
			}
		}
		if (!timeout--) {
			break;
		}
	}

	return result;
}
