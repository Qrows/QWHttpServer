#include "http.h"

#define MIN(a, b) ((a < b) ? a : b)

int init_http_request(struct http_request *req,
                      size_t max_header_number,
                      size_t max_url_size)
{
        if (req == NULL)
                return HTTP_INVALID_INPUT;
        req->method = 0;
        req->url.url = malloc(sizeof(*req->url.url) * (max_url_size + 1));
        if (req->url.url == NULL)
                return HTTP_FAILED_MEM_ALLOCATION;
        req->url.url_max_size = max_url_size;
        req->version = 0;
        req->connection = 0;
        req->headers = create_string_dict(max_header_number);
        if (req->headers == NULL)
                return HTTP_FAILED_MEM_ALLOCATION;
        return HTTP_SUCCESS;
}

void deinit_http_request(struct http_request *req)
{
        if (req == NULL)
                return;
        free(req->url.url);
        destroy_string_dict(req->headers);
}


/* split a str into two, the first start at str and end at the first
 * occurence of token, the second start right after token.
 * all the bytes inside token are transformed in '\0' bytes.
 */
static char *split_str(char *str, char *token)
{
        char *next;
        if (str == NULL || token == NULL)
                return NULL;
        next = strstr(str, token);
        if (next == NULL)
                return NULL;
        for (size_t i = 0; i < strlen(token); i++) {
                *next = '\0';
                ++next;
        }
        return next;
}

static char *get_line(char *str)
{
        return split_str(str, "\r\n");
}

static int parse_method(struct http_request *req, char *method)
{
        int cmp;
        if (req == NULL || method == NULL)
                return HTTP_INVALID_INPUT;
        cmp = strcmp(method, "GET");
        if (cmp == 0) {
                req->method = HTTP_GET;
                return HTTP_SUCCESS;
        }
        cmp = strcmp(method, "HEAD");
        if (cmp == 0) {
                req->method = HTTP_HEAD;
                return HTTP_SUCCESS;
        }
        return HTTP_INVALID_REQUEST_METHOD;
}

static int parse_url(struct http_request *req, char *url)
{
        size_t url_len;
        size_t min_len;
        if (req == NULL || url == NULL)
                return HTTP_INVALID_INPUT;
        url_len = strlen(url);
        if (url_len > req->url.url_max_size)
                return HTTP_URL_TOO_BIG;
        min_len = MIN(url_len, req->url.url_max_size);
        strncpy(req->url.url, url, min_len);
        req->url.url[min_len] = '\0';
        return HTTP_SUCCESS;
}

static int parse_version(struct http_request *req, char *ver)
{
        int cmp;
        if (req == NULL || ver == NULL)
                return HTTP_INVALID_INPUT;
        cmp = strcmp(ver, "HTTP\1.1");
        if (cmp == 0) {
                req->version = HTTP11;
                return HTTP_SUCCESS;
        }
        return HTTP_INVALID_VERSION;
}

static int parse_http_request_line(struct http_request *req, char *req_line)
{
        int err;
        char *curr_token;
        char *remaining;
        /* parse method */
        curr_token = req_line;
        remaining = split_str(curr_token, " ");
        if (remaining == NULL)
                return HTTP_REQUEST_LINE_NOT_FOUND;
        err = parse_method(req, curr_token);
        if (err < 0)
                return err;
        /* parse url */
        curr_token = remaining;
        remaining = split_str(curr_token, " ");
        if (remaining == NULL)
                return HTTP_REQUEST_LINE_NOT_FOUND;
        err = parse_url(req, curr_token);
        if (err < 0)
                return err;
        /* parse version */
        curr_token = remaining;
        err = parse_version(req, curr_token);
        if (err < 0)
                return err;
        return HTTP_SUCCESS;
}

static char *trim_ows(char *str)
{
        while (*str != '\0' && *str == ' ') {
                *str = '\0';
                ++str;
        }
        return str;
}

static void remove_ending_ows(char *str)
{
        /* reach the start of the ending optional whitespace*/
        while (*str != '\0' && *str != ' ')
                ++str;
        /* remove the first trailing ows transforming it in a \0 */
        *str = '\0';
}

static int parse_http_header(struct http_request *req, char *header)
{
        char *name;
        char *value;
        name = header;
        value = split_str(name, ":");
        if (value == NULL)
                return HTTP_INVALID_HEADER;
        value = trim_ows(value);
        remove_ending_ows(value);
        string_dict_add(req->headers, name, value);
        return HTTP_SUCCESS;
}

static bool is_last_line(char *line)
{
        return (strcmp(line, "") == 0);
}

static int parse_http_headers(struct http_request *req, char *headers)
{
        char *curr_line;
        curr_line = headers;
        headers = get_line(curr_line);
        while (!is_last_line(curr_line)) {
                parse_http_header(req, curr_line);
                curr_line = headers;
                headers = get_line(curr_line);
        }
        return 0;
}

int http_parse_request(struct http_request *req, char *to_parse)
{
        int err;
        char *request_line;
        char *headers;
        request_line = to_parse;
        headers = get_line(request_line);
        if (headers == NULL)
                return HTTP_REQUEST_LINE_NOT_FOUND;
        err = parse_http_request_line(req, request_line);
        if (err < 0)
                return err;
        err = parse_http_headers(req, headers);
        if (err < 0)
                return err;
        return HTTP_SUCCESS;
}
