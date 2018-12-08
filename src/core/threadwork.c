#include "threadwork.h"

void *threadwork(void *server_data)
{
	int res = 0;
	struct server_data *data = NULL;
	data = (struct server_data *)server_data;
	if (data->session == NULL || data->data == NULL) {
		syslog(LOG_EMERG, "%s\n",
		       "corrupted server data, can't start listening");
		pthread_exit(NULL);
	}
	while (true) {
		reset_http_session(data->session);
		server_write_info_log(NULL, "starting new connection");
		res = http_start_connection(data->session);
		if (res < 0) {
			server_write_err_log(data->session->connection,
					     "failed starting connection",
					     errno);
			continue;
		}
		server_write_info_log(data->session->connection,
				      "successfully established connection");
		res = do_http_session(data);
		server_write_info_log(data->session->connection,
				      "closing http session");
		http_close_connection(data->session);
	}
	pthread_exit(NULL);
}
