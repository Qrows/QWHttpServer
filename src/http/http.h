#ifndef HTTP_H
#define HTTP_H

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "dictionary.h"

enum http_error {
        HTTP_SUCCESS = 0,
        HTTP_INVALID_INPUT = -1,
        HTTP_FAILED_MEM_ALLOCATION = -2,
        HTTP_REQUEST_LINE_NOT_FOUND = -3,
        HTTP_INVALID_REQUEST_LINE = -4,
        HTTP_INVALID_REQUEST_METHOD = -5,
        HTTP_URL_TOO_BIG = -6,
        HTTP_INVALID_VERSION = -7,
        HTTP_END_CRLF_NOT_FOUND = -8,
        HTTP_INVALID_HEADER = -9
};

/* enum to catalogate the supported method of the http connection */
enum http_method {
        HTTP_GET,
        HTTP_HEAD
};

/* struct to categorize the url, the url should be a null terminated string
 * contained in a buffer of url_max_size size
 */
struct http_url {
        char *url;
        size_t url_max_size;
};

/* enum to catalogate the supported http version */
enum http_version {
        HTTP11
};

/* enum to catalogate the possible value of the header 'connection'*/
enum http_connection {
        HTTP_KEEP_ALIVE,
        HTTP_CLOSE
};

/* http_request struct save all the information of an http request*/
struct http_request {
        enum http_method method;
        struct http_url url;
        enum http_version version;
        struct string_dict *headers;
        enum http_connection connection;
};


/* init_http_request() will allocate all the necessary memory and init the value
 * of an http_request struct.
 */
int init_http_request(struct http_request *req,
                      size_t max_header_number,
                      size_t max_url_size);
/* http_parse_request will parse an http request given as a null terminated
 * string and put the result inside the *req struct
 */
int http_parse_request(struct http_request *req, char *to_parse);

/* deinit_http_request() will deallocate all the resources allocate by
 * init_http_request().
 */
void deinit_http_request(struct http_request *req);
#endif
