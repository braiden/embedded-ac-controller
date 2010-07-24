#pragma once

#include <avr/pgmspace.h>
#include "socket.h"
#include "pff.h"

#ifndef HTTPD_PORT
#define HTTPD_PORT 80
#endif

#ifndef HTTPD_MAX_CLIENTS
#define HTTPD_MAX_CLIENTS 3
#endif

#define HTTPD_GET 1
#define HTTPD_HEAD 2
#define HTTPD_POST 3
#define HTTPD_INVALID_METHOD 4

#define HTTPD_OK 0
#define HTTPD_ERR 1
#define HTTPD_NOTFOUND 2

#define HTTPD_FILENAME_MAX_LENGTH 16

#define HTTPD_READ_DONE 0
#define HTTPD_READ_PATH 1
#define HTTPD_READ_QUERY_STRING 2
#define HTTPD_READ_QUERY_PARAM 3
#define HTTPD_READ_QUERY_VALUE 4

typedef void (*httpd_cgi_handler_t)(uint8_t sockfd, uint8_t method, char *buffer);

void httpd_init();
void httpd_loop(httpd_cgi_handler_t cgi_handler);
void httpd_write_response(uint8_t sockfd, uint8_t status);
uint8_t httpd_read_path(uint8_t sock, char *buffer);
