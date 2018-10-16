#include <stdio.h>
#include <stdlib.h>

#include "http.h"
#include "connection.h"

int main(int argc, char **argv)
{
	LISTENER_CONNECTION *listener = NULL;
	struct http_session *session = NULL;
	struct CONNECTION_attr attr;
	if (argc < 2) {
		fprintf(stderr, "%s port\n", *argv);
		return EXIT_FAILURE;
	}
	listener = lcopen(argv[1], 10, NULL);
	if (listener == NULL) {
		return EXIT_FAILURE;
	}
	attr.write_buffer_size = 64;
	attr.read_buffer_size = 64;
	attr.socket_type = ACCEPT;
	attr.listener = listener;
	attr.addr = NULL;
	attr.addr_len = NULL;
	session = http_session_create(&attr, 60, 128, 128);
	if (session == NULL) {
		return EXIT_FAILURE;
	}
	fprintf(stderr, "%s\n", "http_start_connection()");
	if (http_start_connection(session) < 0) {
		fprintf(stderr, "connection failed!\n");
 		return EXIT_FAILURE;
	}
	fprintf(stderr, "%s\n", "read_http_request()");
	if (read_http_request(session) < 0) {
		fprintf(stderr, "can't read request!\n");
		return EXIT_FAILURE;
	}
	fprintf(stderr, "%s\n", "parse_http_request()");
	if (parse_http_request(session->request, &session->req) < 0) {
		fprintf(stderr, "parser failed\n");
		return EXIT_FAILURE;
	}
	printf("method = %s\n", session->req.method);
	printf("url = %s\n", session->req.url);
	printf("accept = %s\n", session->req.accept);
	printf("connection = %s\n", session->req.connection);
	return EXIT_SUCCESS;
	
}
