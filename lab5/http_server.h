#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <stdbool.h>

bool start_http_server(int port);
void stop_http_server();
void http_handle_request(void *arg);

#endif
