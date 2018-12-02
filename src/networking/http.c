#include "http.h"

struct http_session *http_session_create(struct CONNECTION_attr *attr,
					 int timeout,
					 size_t request_size,
					 size_t response_size)
{
	struct http_session *session = NULL;
	if (attr == NULL) {
		return NULL;
	}
	session = malloc(sizeof(*session));
	if (session == NULL)
		return NULL;
	session->attr = attr;
	session->timeout = timeout;
	session->request = calloc(request_size, sizeof(char));
	if (session->request == NULL) {
		free(session);
		return NULL;
	}
	session->request_size = request_size;
       	session->response = calloc(response_size, sizeof(char));
	if (session->response == NULL) {
		free(session->request);
		free(session);
		return NULL;
	}
	session->response_size = response_size;
	return session;
}

void http_session_destroy(struct http_session *session)
{
	if (session == NULL)
		return;
	free(session->response);
	free(session->request);
	free(session);
}

int http_start_connection(struct http_session *session)
{
	if (session == NULL)
		return -1;
	session->connection = copen(session->attr);
	if (session->connection == NULL)
		return -1;
	return 0;
}

int http_close_connection(struct http_session *session)
{
	if (session == NULL)
		return -1;
	return cclose(session->connection, O_RDWR);
}

void reset_http_session(struct http_session *session)
{
	if (session == NULL)
		return;
	memset(session->request, 0, session->request_size);
	memset(session->response, 0, session->response_size);
	memset(&session->req, 0, sizeof(session->req));
}

int read_http_request(struct http_session *session)
{
	ssize_t rd = 0;
	char *line = NULL;
	size_t read = 0;
	if (session == NULL) {
		return -1;
	}
	read = 0;
	line = session->request;
	while (read < session->request_size) {
		rd = crecvline(session->connection,
			       line,
			       session->request_size - read,
			       session->timeout);
		if (rd < 0) {
			return -1;
		} if (rd == 0) {
			return -1;
		} else {
			if (strcmp(line, CRLF) == 0)
				return 0;
			read += rd;
			line += rd;
		}
	}
	return -1;
}
		      

static int parse_request_line(char *request_line, struct http_request *req)
{
	char *saveptr = NULL;
	req->method = strtok_r(request_line, " ", &saveptr);
	if (req->method == NULL)
		return -1;
	req->url = strtok_r(NULL, " ", &saveptr);
	if (req->url == NULL)
		return -1;
	return 0;
}

static int parse_header(char *header, struct http_request *req)
{
	char *name = NULL;
	char *value = NULL;
	char *ptr = NULL;
	ptr = strchr(header, ':');
	if (ptr == NULL)
		return -1;
	name = header;
	*ptr = '\0';
	value = ptr + 1;
	// trim optioal whitespace
	while (*value != '\0' && *value == ' ')
		++value;
	if (strcmp(name, "Accept") == 0) {
		req->accept = value;
	} else if (strcmp(name, "Connection") == 0) {
		req->connection = value;
	}
	return 0;
}


int parse_http_request(char *raw, struct http_request *req)
{
	char *request_line = NULL;
	char *header = NULL;
	char *raw_saveptr = NULL;
	request_line = strtok_r(raw, CRLF, &raw_saveptr);
	if (request_line == NULL)
		return -1;
	if (parse_request_line(request_line, req) < 0)
		return -1;
	while ((header = strtok_r(NULL, CRLF, &raw_saveptr)) != NULL) {
		parse_header(header, req);
	}
	return 0;
}

double search_weight_from_mime(const char *accept, const char *mime)
{
	char *found = NULL;
	char *column = NULL;
	char *comma = NULL;
	char *error_ptr = NULL;
	double weight = 0;
	if (accept == NULL || mime == NULL)
		return -1;
	found = strstr(accept, mime);
	if (found == NULL)
		return -1;
	column = strchr(found, ';');
	if (column == NULL)
		return -1;
	comma = strchr(found, ',');
	if (comma != NULL && column > comma)
		return -1;
	if (strncmp(column + 1, "q=", 2) != 0)
		return -1;
	weight = strtod(column + 3, &error_ptr);
	if (error_ptr == NULL)
		return -1;
	return weight;
}

bool is_keep_alive(char *connection)
{
	char *KEEP_ALIVE = "Keep-Alive";
	if (connection == NULL)
		return false;
	// trim whitespace
	while (*connection != '\0' && *connection != ' ')
		++connection;
	return strncmp(connection, KEEP_ALIVE, strlen(KEEP_ALIVE)) == 0;
}

static char *get_status_line(enum status_code code)
{
	switch(code) {
	case HTTP_OK:
		return "HTTP/1.1 200 OK\r\n";
	case HTTP_BAD_REQUEST:
		return "HTTP/1.1 402 BAD REQUEST\r\n";
	case HTTP_NOT_FOUND:
		return "HTTP/1.1 404 NOT FOUND\r\n";
	case HTTP_INTERNAL_SERVER_ERROR:
		return "HTTP/1.1 500 INTERNAL SERVER ERROR\r\n";
	default:
		return NULL;
	}
}

static ssize_t write_header(char *buf, size_t buf_size, const char *name, const char *value)
{
	size_t name_len = 0;
	size_t value_len = 0;
	size_t len = 0;
	if (buf == NULL || name == NULL || value == NULL)
		return -1;
	name_len = strlen(name);
	value_len = strlen(value);
	len = name_len + 2 + value_len + CRLF_LEN;
	if (len > buf_size)
		return -1;
	strcpy(buf, name);
	strcat(buf, ": ");
	strcat(buf, value);
	strcat(buf, CRLF);
	return len;
}

static int write_date(char *buf, size_t buf_size)
{
	char date[1024] = {0};
	time_t current_time = 0;
	struct tm *local = NULL;
	int err = 0;
	char *ptr = NULL;
	if (buf == NULL)
		return -1;
	current_time = time(NULL);
	local = gmtime(&current_time);
	err = snprintf(date, 1024, "%s", asctime(local));
	if (err > 1024) // extremely unlikely 
		return -1;
	ptr = strchr(date, '\n');
	if (ptr == NULL) // should never happen
		return -1;
	*ptr = '\0';
	return write_header(buf, buf_size, "Date", date);
}

size_t generate_response_header(char *buf,
				size_t buf_size,
				enum status_code code,
				const char *content_type,
				const char *content_lenght)
{
	char *status_line = NULL;
	size_t len = 0;
	ssize_t err = 0;
	if (buf == NULL)
		return -1;
	status_line = get_status_line(code);
	len = strlen(status_line);
	if (len > buf_size)
		return 0;
	memcpy(buf, status_line, len);
	err = write_date(buf + len, buf_size - len);
	len += err;
	if (err < 0)
		return 0;
	if (content_type) {
		err = write_header(buf + len,
				   buf_size - len,
				   "Content-Type",
				   content_type);
		if (err < 0)
			return 0;
		len += err;
	}

	if (content_lenght) {
		err = write_header(buf + len,
				   buf_size - len,
				   "Content-Lenght",
				   content_lenght);
		if (err < 0)
			return 0;
		len += err;
	}
	// add ending CRLF
	if (len + CRLF_LEN < buf_size) {
		strncpy(buf + len, CRLF, CRLF_LEN);
		len += 2;
		return len;
	} else {
		return 0;
	}
	
	
}
