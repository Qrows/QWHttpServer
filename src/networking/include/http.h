#ifndef HTTP_H
#define HTTP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <syslog.h>
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

enum status_code {
	HTTP_OK = 200,
	HTTP_BAD_REQUEST = 402,
	HTTP_NOT_FOUND = 404,
	HTTP_INTERNAL_SERVER_ERROR = 500
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
/**
 * http_session_create - alloc and init a new http_session struct
 * @attr: CONNECTION_attr struct to create a working CONNECTION
 * @timeout: timeout for the client, if no data is 
 * sent in the last timeout seconds, end the session
 * @request_size: max size of a request line
 * @response_size: max size of a response line
 */
struct http_session *http_session_create(struct CONNECTION_attr *attr,
	                                 int timeout,
					 size_t request_size,
					 size_t response_size);
/**
 * http_session_destroy - free all the resource allocated by 
 * http_session_create()
 * @session: http_session struct to free
 */
void http_session_destroy(struct http_session *session);

/**
 * http_start_connection - start listening for incoming connection
 * @session: struct containing http session state,
 * the struct should not be already connected.
 * this call will block untill a nxew connection is opened.
 */
int http_start_connection(struct http_session *session);

/**
 * http_close_connection - close the CONNECTION opened by http_start_connection()
 * @session: struct containing http session state, should be
 * already connected.
 */
int http_close_connection(struct http_session *session);

/**
 * read_http_request - read an http request line by line till CRLF is found
 * @session: connected http_session struct
 */
int read_http_request(struct http_session *session);

/**
 * parse_http_request - parse a raw http request
 * @raw: string containing an http request
 * @req: struct where to store the information retrieved
 */
int parse_http_request(char *raw, struct http_request *req);

/**
 * search_weight_from_mime - given the accept header and a mime, return the weight
 * @accept: value of the header accept
 * @mime: mime value
 */
double search_weight_from_mime(const char *accept, const char *mime);

/**
 * is_keep_alive - given the value of the header Connection,
 * return true if keep-alive is requested.
 * @connection: value of the header connection
 * given RFC 7230 6.3 if not specified (NULL) return true 
 */
bool is_keep_alive(char *connection);

/**
 * generate_response_header - generate an http response header
 * @response: buffer where to store the response header
 * @response_size: size of the response buffer
 * @code: status code of the response
 * @content_type: string containing the mime of the resource
 * @content_lenght: the lenght of the resource
 */
size_t generate_response_header(char *response,
				size_t response_size,
				enum status_code code,
				const char *content_type,
				const char *content_lenght);

/**
 * reset_http_session - reset an http session to a fresh
 * valid state without destroing it.
 * @session: http_session to reset
 */
void reset_http_session(struct http_session *session);
#endif
