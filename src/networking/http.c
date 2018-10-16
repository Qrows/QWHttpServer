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
	// todo trim optioal whitespace
	if (strcmp(name, "Accept") == 0) {
		req->accept = value;
	}
	if (strcmp(name, "Connection") == 0) {
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

double search_weight_from_mime(char *accept, char *mime)
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
	// trim whitespace
	while (*connection != '\0' && *connection != ' ')
		++connection;
	return strncmp(connection, KEEP_ALIVE, strlen(KEEP_ALIVE)) == 0;
}
