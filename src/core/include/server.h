#ifndef SERVER_H
#define SERVER_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <syslog.h>
#include <pthread.h>
#include <errno.h>
#include <limits.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "connection.h"
#include "http.h"
#include "file_system.h"
#include "image_processing.h"

struct global_server_data {
	struct content_proxy *cp;
};

struct server_data {
	pthread_t pid;
	struct global_server_data *data;
	struct http_session *session;
};

struct server_data *server_data_create(void);
void server_data_destroy(struct server_data *sd);
struct global_server_data *create_global_server_data(char *root, char *cache, char *index);
void destroy_global_server_data(struct global_server_data *gsd);

int do_http_session(struct server_data *data);

void server_open_log(bool to_console, bool to_stderr);
void server_close_log(void);
void server_write_err_log(const CONNECTION *connection, const char *str, int errnum);
void server_write_info_log(const CONNECTION *connection , const char *str);	

#endif
