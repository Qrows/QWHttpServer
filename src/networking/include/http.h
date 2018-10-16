#ifndef HTTP_H
#define HTTP_H

#include <stdlib.h>
#include "connection.h"

#ifndef CRLF
#define CRLF "\r\n"
#endif
#ifndef CRLF_LEN
#define CRLF_LEN 2
#endif

struct http_request {
	char *method;
	char *url;
	char *accept;
	char *connection;
};

struct http_session {
	struct CONNECTION_attr *attr;
	CONNECTION *connection;
	int timeout;
	char *request;
	size_t request_size;
	struct http_request req;
	char *response;
	size_t response_size;
};

struct http_session *http_session_create(struct CONNECTION_attr *attr,
	                                 int timeout,
					 size_t request_size,
					 size_t response_size);
void http_session_destroy(struct http_session *session);
int http_start_connection(struct http_session *session);
int read_http_request(struct http_session *session);
int parse_http_request(char *raw, struct http_request *req);
double search_weight_from_mime(char *accept, char *mime);

#endif
