#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http.h"
#include "connection.h"

int main(int argc, char *argv[])
{
	LISTENER_CONNECTION *listener = NULL;
	struct http_session *session = NULL;
	struct CONNECTION_attr attr = {0};
	size_t resp_size = 0;
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
	if (http_start_connection(session) < 0) {
		fprintf(stderr, "%s\n", "connection failed!\n");
 		return EXIT_FAILURE;
	}
	if (read_http_request(session) < 0) {
		fprintf(stderr, "%s\n", "can't read request!\n");
		return EXIT_FAILURE;
	}
	if (parse_http_request(session->request, &session->req) < 0) {
		fprintf(stderr, "%s\n", "parser failed");
		return EXIT_FAILURE;
	}
	resp_size = generate_response_header(session->response,
					     session->response_size,
					     HTTP_OK,
					     "text/plain",
					     NULL);
	printf("%s\n", session->response);
	if (resp_size == 0) {
		fprintf(stderr, "%s\n", "failed generating response header!");
		return EXIT_FAILURE;		
	}
	csend(session->connection, session->response, resp_size);
	return EXIT_SUCCESS;
}
