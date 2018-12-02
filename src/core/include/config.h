#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "server.h"

struct config {
	size_t connection_write_buffer;
	size_t connection_read_buffer;
	char *server_root;
	char *server_cache;
	int thread_number;
	int backlog;
	size_t http_max_request_size;
	size_t http_max_response_size;
	char *port;
	int timeout;
};

/**
 * create_config() - parse cfg_file and create a struct config with it
 * @cfg_file: path to the config file
 */
struct config *create_config(char *cfg_file);
#endif
