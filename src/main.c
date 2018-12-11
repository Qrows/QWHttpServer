
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http.h"
#include "connection.h"
#include "server.h"
#include "config.h"
#include "threadwork.h"

static void close_listening_socket(LISTENER_CONNECTION *listener)
{
	if (listener == NULL)
		return;
	syslog(LOG_INFO, "%s\n", "closening listening socket");
	lcclose(listener);
	syslog(LOG_INFO, "%s\n", "listening socket successfully closed");
}

int main(int argc, char *argv[])
{
	struct config *cfg = NULL;
	LISTENER_CONNECTION *listener = NULL;
	struct CONNECTION_attr *attr = NULL;
	struct server_data **data = NULL;
	struct global_server_data *global_data = NULL;
	int res = 0;
	if (argc < 2 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
		fprintf(stderr, "%s %s\n", *argv,
			"path-to-config-file");
		return EXIT_FAILURE;
	}
	server_open_log(true, true);
	syslog(LOG_INFO, "%s %s\n", "reading config at", argv[1]);
	cfg = create_config(argv[1]);
	if (cfg == NULL) {
		syslog(LOG_CRIT, "%s\n", "error in config file");
		return EXIT_FAILURE;
	}
	syslog(LOG_INFO, "%s\n", "config file succesfully readed");
	syslog(LOG_INFO, "%s %s %s %d\n",
	       "opening socket at port",
	       cfg->port,
	       "with backlog",
	       cfg->backlog);
	res = cmasksigpipe();
	if (res < 0) {
		syslog(LOG_CRIT, "%s: %s\n", "cant mask SIGPIPE",
		       strerror(errno));
		return EXIT_FAILURE;
	}
		
	listener = lcopen(cfg->port, cfg->backlog, NULL);
	if (listener == NULL) {
		syslog(LOG_CRIT, "%s %s\n", "can't open listening socket at port", cfg->port);
		return EXIT_FAILURE;
	}
	syslog(LOG_INFO, "%s\n", "socket succesfully open");

	attr = calloc(1, sizeof(*attr));
	if (attr == NULL) {
		syslog(LOG_CRIT, "%s\n", "can't allocate server structure");
		return EXIT_FAILURE;
	}
	attr->write_buffer_size = cfg->connection_write_buffer;
	attr->read_buffer_size = cfg->connection_read_buffer;
	attr->socket_type = ACCEPT;
	attr->listener = listener;
	attr->addr = NULL;
	attr->addr_len = NULL;
	
	global_data = create_global_server_data(cfg->server_root,
						cfg->server_cache,
						cfg->server_index);
	if (global_data == NULL) {
		syslog(LOG_EMERG, "%s\n", "can't create global server data");
		close_listening_socket(listener);
		return EXIT_FAILURE;
	}
	data = calloc(cfg->thread_number, sizeof(*data));
	if (data == NULL) {
		syslog(LOG_EMERG, "%s\n", "can't create server data");
		close_listening_socket(listener);
		return EXIT_FAILURE;
	}
	for (int i = 0; i < cfg->thread_number; ++i) {
		data[i] = server_data_create();
		if (data[i] == NULL) {
			return EXIT_FAILURE;
		}
		data[i]->data = global_data;
		data[i]->session = http_session_create(attr,
						       cfg->timeout,
						       cfg->http_max_request_size,
						       cfg->http_max_response_size);
		if (data[i]->session == NULL) {
			syslog(LOG_EMERG, "%s\n", "can't create http session");
			close_listening_socket(listener);
			return EXIT_FAILURE;
		}
	}
	syslog(LOG_INFO, "%s %d %s\n", "starting",
	       cfg->thread_number,
	       "working server thread");
	for (int i = 0; i < cfg->thread_number; ++i) {
 		res = pthread_create(&data[i]->pid, NULL, threadwork, data[i]);
		if (res != 0) {
			syslog(LOG_EMERG, "%s\n", "can't create worker thread");
			close_listening_socket(listener);
			return EXIT_FAILURE;
		}
	}
	
	for (int i = 0; i < cfg->thread_number; ++i) {
		res = pthread_join(data[i]->pid, NULL);
		if (res != 0) {
			syslog(LOG_EMERG, "%s\n", "can't join the thread");
			close_listening_socket(listener);
			return EXIT_FAILURE;
		}
	}
	close_listening_socket(listener);
	return EXIT_SUCCESS;
}
