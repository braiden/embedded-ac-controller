#include <stdlib.h>
#include <string.h>
#include "httpd.h"
#include "socket.h"
#include "uart.h"
#include "w5100.h"
#include "romfs.h"
#include "debug.h"

uint8_t _listen_sock;
uint8_t _clients[HTTPD_MAX_CLIENTS];
uint8_t _connected_clients;
uint8_t _fdset;

void httpd_init()
{
	// create listening socket
	_listen_sock = sock_listen(HTTPD_PORT, HTTPD_MAX_CLIENTS);
	// no connected clients
	_connected_clients = 0;
	_fdset = 0x00;
	// add listening socket to our FD set (used by select)
	FD_SET(_fdset, _listen_sock);
}

void httpd_write_response(uint8_t sockfd, uint8_t status)
{
	sock_write_P(sockfd, PSTR("HTTP/1.1 "), 9);

	if (status == HTTPD_OK) {
		sock_write_P(sockfd, PSTR("200 OK\r\n"), 8);
		log("200\n");
	} else {
		if (status == HTTPD_NOTFOUND) {
			sock_write_P(sockfd, PSTR("404 NOT FOUND\r\n"), 15);
			log("404\n");
		} else {
			sock_write_P(sockfd, PSTR("500 SERVER ERROR\r\n"), 11);
			log("500\n");
		}
		sock_write_P(sockfd, PSTR("Content-Type: text/html\r\n\r\n"), 27);
		if (status == HTTPD_NOTFOUND) {
			sock_write_P(sockfd, PSTR("Not Found"), 9);
		} else  {
			sock_write_P(sockfd, PSTR("Server Error"), 12);
		}
	}
}

static uint8_t _httpd_read_method(uint8_t sock)
{
	char buffer[2];

	if (sock_read(sock, buffer, 2)) {

#ifdef DEBUG
		uint8_t n;
		for (n=0; n<4; n++) {
			log_int(w5100_mem_read(W5100_SOCKET_REG+sock, W5100_Sn_DIPR0+n),1,10);
			if (n<3) log_char('.');
		}
#endif

		if (!strncmp_P(buffer, PSTR("GE"), 2)) {
			buffer[0] = HTTPD_GET;
			buffer[1] = 3;
			log(" GET\n");
		} else if (!strncmp_P(buffer, PSTR("HE"), 2)) {
			buffer[0] = HTTPD_HEAD;
			buffer[1] = 4;
			log(" HEAD\n");
		} else if (!strncmp_P(buffer, PSTR("PO"), 2)) {
			buffer[0] = HTTPD_POST;
			buffer[1] = 4;
			log(" POST\n");
		} else {
			buffer[0] = HTTPD_INVALID_METHOD;
			buffer[1] = 0;
			log(" ???\n");
		}

		sock_read(sock, NULL, buffer[1]);
		return buffer[0];
	}

	return HTTPD_INVALID_METHOD;
}

uint8_t httpd_read_path(uint8_t sock, char *buffer)
{
	uint8_t maxlen = HTTPD_FILENAME_MAX_LENGTH;
	char c;

	// read from stream until connection gone, buffer full, or end of token
	while (sock_read(sock, &c, 1)
			&& c != '/' && c != ' ' && c != '\n' && c != '\r'
			&& c != '?' && c != '&' && c != '='
			&& maxlen--) {
		*buffer++ = c;
		log_char(c);
	}
	*buffer = 0;
	log("\n");

	if (c == ' ' || c == '\n' || c == '\r') {
		// no more tokens
		return HTTPD_READ_DONE;
	} else if (c =='/') {
		// more tokens
		return HTTPD_READ_PATH;
	} else if (c == '?') {
		// no more tokens, but query string exists
		return HTTPD_READ_QUERY_STRING;
	} else if (c == '&') {
		// start of query string param
		return HTTPD_READ_QUERY_PARAM;
	} else if (c == '=') {
		// query string value
		return HTTPD_READ_QUERY_VALUE;
	} else if (maxlen == 0) {
		// url is too long, won't continue parsing
		return HTTPD_READ_DONE;
	} else {
		// loop exitted for another reason, client gone?
		return HTTPD_READ_DONE;
	}

}

static void _httpd_write_content_type(uint8_t sock, char *filename)
{
	PGM_P type = NULL;

	sock_write_P(sock, PSTR("Content-Type: "), 14);

	if (!*filename) {
		// no filname in url, that mean a direcotry was openned with
		// no file, and the deafult (empty) filename was picked up
		// assumit it is .html
		type = PSTR("text/html");
	} else {
		// position pointer at first char after .
		while (*filename && *filename++ != '.');

		if (strcmp_P(filename, PSTR("html")) == 0) {
			type = PSTR("text/html");
		} else if (strcmp_P(filename, PSTR("txt")) == 0) {
			type = PSTR("text/plain");
		} else if (strcmp_P(filename, PSTR("jpg")) == 0) {
			type = PSTR("image/jpeg");
		} else if (strcmp_P(filename, PSTR("gif")) == 0) {
			type = PSTR("image/gif");
		} else if (strcmp_P(filename, PSTR("css")) == 0) {
			type = PSTR("text/css");
		} else if (strcmp_P(filename, PSTR("js")) == 0) {
			type = PSTR("application/x-javascript");
		} else {
			type = PSTR("application/octet-stream");
		}
	}

	sock_write_P(sock, type, strlen_P(type));
	sock_write_P(sock, PSTR("\r\n"), 2);
}

static void _httpd_handle_uri(uint8_t sock, uint8_t method, httpd_cgi_handler_t cgi_handler)
{
	uint8_t status;
	// open the root node of filesystem
	romfs_node_t dirnode, filenode;
	romfs_root(&dirnode);
	
	char buffer[HTTPD_FILENAME_MAX_LENGTH + 3];
	while ((status = httpd_read_path(sock, buffer))) {
		if (status == HTTPD_READ_PATH) {
			// special directory 'cgi-bin' delegates control to external function
			if (strcmp_P(buffer, PSTR("cgi-bin")) == 0) {
				(*cgi_handler)(sock, method, buffer);
				return;
			// try to open the path as a directory
			} else if (romfs_open(&dirnode, &filenode, buffer)) {
				if (!ROMFS_IS_DIRECTORY(filenode)) {
					// the url requested a directory, but a file
					// with that name exists, resource not found
					break;
				}
				// use the new node as our directory
				dirnode = filenode;
			} else {
				// diretory could not be openned
				break;
			}
		} else if (status == HTTPD_READ_QUERY_STRING) {
			// stop processing tokens at query string
			status = HTTPD_READ_DONE;
			break;
		}
	}

	// status == 0, we stopped tokenizing the url and buffer
	// contains a filename, try to open it
	if (!status) {
		// try to open file
		status = romfs_open(&dirnode, &filenode, buffer);

		if (status) {
			// file was found, write headers
			httpd_write_response(sock, HTTPD_OK);
			_httpd_write_content_type(sock, buffer);
			sock_write_P(sock, PSTR("\r\n"), 2);
		} else {
			// try to open the .gz version of the file
			strcat_P(buffer, PSTR(".gz"));
			log("Trying .gz\n");
			status = romfs_open(&dirnode, &filenode, buffer);
			buffer[strlen(buffer)-3] = 0x00;
			if (status) {
				// found a gzip'd version
				httpd_write_response(sock, HTTPD_OK);
				_httpd_write_content_type(sock, buffer);
				sock_write_P(sock, PSTR("Content-Encoding: gzip\r\n\r\n"), 26);
			} else {
				// 404
				httpd_write_response(sock, HTTPD_NOTFOUND);
			}
		}

		if (status) {
			// we were able to open the file, send it
			// it not head, write the data
			if (method != HTTPD_HEAD) {
				uint16_t len = filenode.data_len;
				PGM_VOID_P data = filenode.data;
				// sock_write can only do 255 bytes at a time
				while (len) {
					if (len > 255) {
						sock_write_P(sock, data, 255);
						data += 255;
						len -= 255;
					} else {
						sock_write_P(sock, data, len);
						len = 0;
					}
				}
			}
		}
	} else {
		httpd_write_response(sock, HTTPD_NOTFOUND);
	}
}

void httpd_loop(httpd_cgi_handler_t cgi_handler)
{
	// wait for something to happen
	uint8_t ready = sock_select(_fdset, 0x7F);

	// new client is connecting
	if (FD_ISSET(ready, _listen_sock)) {
		uint8_t new_client = sock_accept(_listen_sock);
		_clients[_connected_clients++] = new_client;
		FD_SET(_fdset, new_client);
	}

	uint8_t n = 0;
	for (n = 0; n < _connected_clients; n++) {
		uint8_t fd = _clients[n];
		// a connected client is readable
		if (FD_ISSET(ready, fd)) {
			uint8_t method = _httpd_read_method(fd);
			if (method != HTTPD_INVALID_METHOD) {
				// invoke the handler, fd's next read will read first char of url
				_httpd_handle_uri(fd, method, cgi_handler);
			} else {
				httpd_write_response(fd, HTTPD_ERR);
			}
			// we always close connection
			sock_close(fd);
			FD_CLEAR(_fdset, fd);
			_clients[n] = _clients[_connected_clients--];
			n--;
		}
	}
}
