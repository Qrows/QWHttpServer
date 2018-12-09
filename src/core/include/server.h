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


/**
 * server_data_create - allocate and init a server_data struct
 */
struct server_data *server_data_create(void);
/**
 * server_data_destroy - free all resource allocated by server_data_create()
 * @sd: server_data struct to free
 */
void server_data_destroy(struct server_data *sd);
/**
 * create_global_server_data - allocate and init a global_server_data struct
 * @root: root folder path of the content proxy
 * @cache: cache folder path of the content proxy
 * @index: index file, to get retrieved when '/' is asked
 */
struct global_server_data *create_global_server_data(char *root, char *cache, char *index);
/**
 * destroy_global_server_data - free all resource allocated by 
 * create_global_server_data_struct()
 * @gsd: global_server_data struct to free
 */
void destroy_global_server_data(struct global_server_data *gsd);

/**
 * do_http_session - execute a http session
 * @data: struct containing the information about the current session
 * this function does:
 * 1. read incoming message
 * 2. parse http request
 * 3. execute the parsed request
 * 4. send the result
 */
int do_http_session(struct server_data *data);

/**
 * server_open_log - open the system logger
 * @to_console:  send the log to console if true
 * @to_stderr: send the log to stderr if true
 */
void server_open_log(bool to_console, bool to_stderr);

/**
 * server_close_log - close system log opened by server_open_log()
 */
void server_close_log(void);
/**
 * server_write_err_log - write error msg of format "[%ip]:%errstr: %str"
 * @connection: Connection for retrieving the ip address of the client
 * @str: error msg
 * @errnum: errno number to get errno string msg
 */
void server_write_err_log(const CONNECTION *connection, const char *str, int errnum);
/**
 * server_write_info_log - write info msg of format "[%ip] %
 * @connection: Connection for retrieving the ip address of the client 
 * @str: info msg
 */
void server_write_info_log(const CONNECTION *connection , const char *str);	

#endif
