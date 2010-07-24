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
