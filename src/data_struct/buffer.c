#include "buffer.h"

struct buffer *buffer_create(size_t size)
{
	struct buffer *buf = NULL;
	buf = malloc(sizeof(*buf));
	if (buf == NULL)
		return buf;
	buf->data = calloc(size, sizeof(*buf->data));
	if (buf->data == NULL) {
		free(buf);
		return NULL;
	}
	buf->size = size;
	buf->used = 0;
	return buf;
}

void buffer_destroy(struct buffer *buf)
{
	if (buf == NULL)
		return;
	free(buf->data);
	free(buf);
}

void buffer_reset(struct buffer *buf)
{
	if (buf == NULL)
		return;
	memset(buf->data, 0, buf->size);
	buf->used = 0;
}

int buffer_append_data(struct buffer *buffer, char *data, size_t data_size)
{
	size_t data_left = 0;
	if (buffer == NULL || buffer->data == NULL || data == NULL) {
		errno = EINVAL;
		return -1;
	}
	data_left = buffer->size - buffer->used;
	if (data_size > data_left) {
		errno = ENOMEM;
		return -1;
	}
	/* if data overlap with buffer->data */
	if (data < buffer->data + buffer->size && data > buffer->data)
		memmove(buffer->data + buffer->used, data, data_size);
	else
		memcpy(buffer->data + buffer->used, data, data_size);
	buffer->used += data_size;
	return 0;
}

char *buffer_get_data(struct buffer *buffer)
{
	if (buffer == NULL) {
		errno = EINVAL;
		return  NULL;
	} else {
		return buffer->data;
	}
}

char *buffer_get_data_end(struct buffer *buffer)
{
	if (buffer == NULL) {
		errno = EINVAL;
		return  NULL;
	} else {
		return buffer->data + buffer->used;
	}
	
}

size_t buffer_get_size(struct buffer *buffer)
{
	if (buffer == NULL) {
		errno = EINVAL;
		return 0;
	}
	return buffer->size;
}

size_t buffer_get_used_size(struct buffer *buffer)
{
	if (buffer == NULL) {
		errno = EINVAL;
		return 0;
	}
	return buffer->used;
}

size_t buffer_get_size_left(struct buffer *buffer)
{
	if (buffer == NULL) {
		errno = EINVAL;
		return 0;
	}
	return buffer->size - buffer->used;
}

int buffer_increase_used_size(struct buffer *buffer, size_t inc)
{
	if (buffer == NULL) {
		errno = EINVAL;
		return -1;
	}
	if (buffer->used + inc < buffer->size) {
		buffer->used += inc;
		return 0;
	} else {
		errno = EINVAL;
		return -1;
	}
}

void buffer_print(struct buffer *buffer)
{
	if (buffer == NULL || buffer->data == NULL) {
		printf("(Null)\n");
		return;
	}
	for (size_t i = 0; i < buffer->used; i++) {
		putchar(buffer->data[i]);
	}
	putchar('\n');
}
