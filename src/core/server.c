#include "server.h"

#define IS_GET(method) ((strcmp(method, "GET")) == 0)
#define IS_HEAD(method) ((strcmp(method, "HEAD")) == 0)

struct server_data *server_data_create(void)
{
	struct server_data *sd = NULL;
	sd = malloc(sizeof(*sd));
	if (sd == NULL)
		return NULL;
	sd->pid = 0;
	sd->data = NULL;
	sd->session = NULL;
	return sd;
}
void server_data_destroy(struct server_data *sd)
{
	if (sd == NULL)
		return;
	free(sd);
}

struct global_server_data *create_global_server_data(char *root, char *cache)
{
	struct global_server_data *gsd = NULL;
	struct content_proxy_settings cps = {0};
	if (root == NULL || cache == NULL)
		return NULL;
	gsd = malloc(sizeof(*gsd));
	if (gsd == NULL)
		return NULL;
	cps.root = root;
	cps.cache = cache;
	gsd->cp = create_content_proxy(&cps);
	if (gsd->cp == NULL) {
		free(gsd);
		return NULL;
	}
	return gsd;
}

void destroy_global_server_data(struct global_server_data *gsd)
{
	if (gsd == NULL)
		return;
	destroy_content_proxy(gsd->cp);
	free(gsd);
}

static int send_header(struct http_session *session,
		       enum status_code code,
		       const char *mime,
		       const char *content_lenght)
{
	size_t wr = 0;
	ssize_t sd = 0;
	wr = generate_response_header(session->response,
				      session->response_size,
				      code,
				      mime,
				      content_lenght);
	if (wr == 0)
		return -1;
	sd = csend(session->connection, session->response, wr);
	if (sd < 0)
		return -1;
	// flush to send header
	if (cflush(session->connection) < 0)
		return -1;
	return 0;
}

static int send_content(CONNECTION *connection, struct file_data *file)
{
	ssize_t sd = 0;
	sd = csendfile(connection, file->path, file->data_size);
	if (sd < 0)
		return -1;
	return 0;
}

static int size_t_to_str(char *buf, size_t buf_size, size_t sz)
{
	int res = 0;
	res = snprintf(buf, buf_size, "%zu", sz);
	if (res < 0)
		return -1;
	if ((size_t) res > buf_size)
		return -1;
	return 0;
}
static int get_quality(double weight)
{
	if (weight < 0.0 || weight > 1.0)
		return -1;
	return (int)(weight * 100);
}



static int send_file(struct http_session *session,
		     struct file_data *content,
		     char *content_lenght,
		     bool is_get)
{
	int res = 0;
	res = send_header(session,
			  HTTP_OK,
			  content->mime,
			  content_lenght);
	if (res < 0)
		return -1;
	if (is_get) {
		res = send_content(session->connection,
				   content);
		if (res < 0)
			return -1;
	}
		return 0;
}

static int get_accept_weight(const char *mime, const char *accept)
{
	double dweight = 0.0;
	int weight = 0;
	if (mime == NULL || accept == NULL)
		return -1;
	dweight = search_weight_from_mime(accept, mime);
	weight = get_quality(dweight);
	return weight;
}

static int _get_internal(struct server_data *data, bool is_get)
{
	struct file_data *content = NULL;
	struct content_proxy *cp = NULL;
	char *url = NULL;
	int res = 0;
	char content_lenght[64] = {0};
	struct http_request *req = NULL;
	cp = data->data->cp;
	req = &data->session->req;
	url = req->url;
	content = get_content(cp, url, req->accept);
	if (content == NULL) {
		res = send_header(data->session, HTTP_NOT_FOUND, NULL, NULL);
		return res;
	}
	res = size_t_to_str(content_lenght, 63, content->data_size);
	if (res < 0) {
		send_header(data->session, HTTP_INTERNAL_SERVER_ERROR, NULL, NULL);
		return res;
	}
	res = send_file(data->session, content, content_lenght, is_get);
	if (res < 0) { // here if failed to send 
		return res;
	}
	return 0;
}

static int do_head(struct server_data *data)
{
	return _get_internal(data, /*is_get:*/false);
}

static int do_get(struct server_data *data)
{
	return _get_internal(data, /*is_get:*/true);
}

static int do_request(struct server_data *data)
{
	char *method = NULL;
	if (data == NULL)
		return -1;
	if (data->session == NULL || data->data == NULL)
		return -1;
	if (data->session->req.method == NULL)
		return -1;
	method = data->session->req.method;
	if (IS_GET(method)) {
		server_write_info_log(data->session->connection, "client send GET request");
		return do_get(data);
	} else if (IS_HEAD(method)) {
		server_write_info_log(data->session->connection, "client send HEAD request");
		return do_head(data);
	} else {
		return -1;
	}
}

static bool check_connection(struct http_request *req)
{
	if (req->connection == NULL)
		return false;
	return strcmp(req->connection, "keep-alive") == 0;
}

int do_http_session(struct server_data *data)
{
	bool keep_alive = true;
	int res = 0;
	if (data == NULL)
		return -1;
	while (keep_alive) {
		res = read_http_request(data->session);
		if (res < 0) {
			server_write_err_log(data->session->connection,
					     "failed reading http request");
			return -1;
		}
		res = parse_http_request(data->session->request, &data->session->req);
		if (res < 0) {
			send_header(data->session, HTTP_BAD_REQUEST, NULL, NULL);
			server_write_err_log(data->session->connection,
					     "client send bad request");
			return -1;
		}
		res = do_request(data);
		if (res < 0) {
			server_write_err_log(data->session->connection,
					     "can't complete client request");
			return -1;
		}
		syslog(LOG_DEBUG, "%s: %s\n", "Connection", data->session->req.connection);
		keep_alive = check_connection(&data->session->req);
		syslog(LOG_DEBUG, "%s: %s\n", "Keep-Alive", (keep_alive != 0) ? "true" : "false");
		reset_http_session(data->session);
	}
	return 0;
}

void server_open_log(bool to_console, bool to_stderr)
{
	int flags = 0;
	if (to_console)
		flags |= LOG_CONS;
	if (to_stderr)
		flags |= LOG_PERROR;
	openlog("QWHttpServer", flags, LOG_USER);
}

void server_close_log(void)
{
	closelog();
}

void server_write_err_log(const CONNECTION *connection, const char *str)
{
	struct sockaddr ip = {0};
        socklen_t ip_len = 0;
	int res = 0;
	if (connection == NULL || str == NULL)
		return;
	ip_len = sizeof(ip);
	res = cgetpeername(connection, &ip, &ip_len);
	if (res < 0) {
		syslog(LOG_ERR, "[%s] %s\n", "Client Unrecognized ip", str);
	}
	if (ip.sa_family == AF_INET) {
		struct sockaddr_in *ipv4 = (struct sockaddr_in *)&ip;
		char s_ipv4[INET_ADDRSTRLEN] = {0};
		inet_ntop(AF_INET, &ipv4->sin_addr, s_ipv4, INET_ADDRSTRLEN);
		syslog(LOG_ERR, "[%s:%s] %s\n", "Client ipv4", s_ipv4, str);
		
	} else if (ip.sa_family == AF_INET6) {
		struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)&ip;
		char s_ipv6[INET6_ADDRSTRLEN] = {0};
		inet_ntop(AF_INET6, &ipv6->sin6_addr, s_ipv6, INET6_ADDRSTRLEN);
		syslog(LOG_ERR, "[%s:%s] %s\n", "Client ipv6", s_ipv6, str);
	} else {
		syslog(LOG_ERR, "[%s] %s\n", "Client not ipv4 or ipv6", str);
	}
}

void server_write_info_log(const CONNECTION *connection , const char *str)
{
	struct sockaddr ip = {0};
        socklen_t ip_len = 0;
	int res = 0;
	if (connection == NULL || str == NULL)
		return;
	ip_len = sizeof(ip);
	res = cgetpeername(connection, &ip, &ip_len);
	if (res < 0) {
		syslog(LOG_INFO, "[%s] %s\n", "Client Unrecognized ip", str);
	}
	if (ip.sa_family == AF_INET) {
		struct sockaddr_in *ipv4 = (struct sockaddr_in *)&ip;
		char s_ipv4[INET_ADDRSTRLEN] = {0};
		inet_ntop(AF_INET, &ipv4->sin_addr, s_ipv4, INET_ADDRSTRLEN);
		syslog(LOG_INFO, "[%s:%s] %s\n", "Client ipv4", s_ipv4, str);
		
	} else if (ip.sa_family == AF_INET6) {
		struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)&ip;
		char s_ipv6[INET6_ADDRSTRLEN] = {0};
		inet_ntop(AF_INET6, &ipv6->sin6_addr, s_ipv6, INET6_ADDRSTRLEN);
		syslog(LOG_INFO, "[%s:%s] %s\n", "Client ipv6", s_ipv6, str);
	} else {
		syslog(LOG_INFO, "[%s] %s\n", "Client not ipv4 or ipv6", str);
	}
}
