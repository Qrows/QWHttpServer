#ifndef BUFFER_H
#define BUFFER_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


struct buffer {
	char *data;
	size_t size;
	size_t used;
};

struct buffer *buffer_create(size_t size);
void buffer_destroy(struct buffer *buf);
void buffer_reset(struct buffer *buf);
int buffer_append_data(struct buffer *buffer, char *data, size_t data_size);
char *buffer_get_data(struct buffer *buffer);
char *buffer_get_data_end(struct buffer *buffer);
size_t buffer_get_size(struct buffer *buffer);
size_t buffer_get_size(struct buffer *buffer);
size_t buffer_get_used_size(struct buffer *buffer);
size_t buffer_get_size_left(struct buffer *buffer);
int buffer_increase_used_size(struct buffer *buffer, size_t inc);
void buffer_print(struct buffer *buffer);
#endif
