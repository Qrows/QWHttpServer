#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>
#include <pthread.h>
#include <magic.h>

#include "http.h"
#include "image_processing.h"
#include "string_utils.h"

/**
 * file_data - rappresent a file
 * @path: path of the file on the filesystem 
 * @data_size: size of the file
 * @list: linked list entry
 */
struct file_data {
	char *path;
	size_t data_size;
	char *mime;
};

/**
 * content_proxy - thread-safe proxy to access content on file system
 * @root: root on the filesystem where to search the file
 * @cache: where to cache the file
 * @mutex: mutex to sync access to cache
 */
struct content_proxy {
	char *root;
	char *cache;
	char *index;
	pthread_mutex_t mutex;
};

struct content_proxy_settings {
	char *root;
	char *cache;
	char *index;
};

/**
 * create_file_data - alloc and init a file data struct
 * @path: pathname of the file, it will be copied
 * given a pathname return is size, mime and path
 */
struct file_data *create_file_data(const char *path);
/**
 * destroy_file_data - dealloc all the reource allocated by create_file_data()
 * @entry: struct file_data to destroy
 */
void destroy_file_data(struct file_data *entry);

/**
 * create_content_proxy - will alloc on the heap a content_proxy struct
 * @settings: struct containing setting for the content_proxy
 */
struct content_proxy *create_content_proxy(struct content_proxy_settings *cps);

/**
 * destroy_content_proxy - free all the resource allocated by 
 * create_content_proxy()
 * @cp: struct to destroy
 */
void destroy_content_proxy(struct content_proxy *cp);

/**
 * get_file_data - thread-safe content retriever by url
 * @cp: content_proxy, manage the access to the file system
 * @url: url of the requested resource
 */
struct file_data *get_file_data(struct content_proxy *cp, char *url, bool from_cache);
struct file_data *get_content(struct content_proxy *cp, char *url, char *accept);


#endif
