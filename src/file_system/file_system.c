#include "file_system.h"

/**
 * get_mime_string - get the mime type of a file using the magic number
 * @file: path of the file to get the mime types
 * return a string allocated on the heap with malloc(2)
 */
char *get_mime_string(const char *file)
{
	magic_t mcp = NULL;
	const char *tmp = NULL;
	int err = 0;
	char *mime_str = NULL;
	size_t tmp_len = 0;
	if (file == NULL)
		return NULL;
	mcp = magic_open(MAGIC_MIME);
	if (mcp == NULL)
		return NULL;
	err = magic_load(mcp, NULL);
	if (err < 0) {
		magic_close(mcp);
		return NULL;
	}
	tmp = magic_file(mcp, file);
	if (tmp == NULL) {
		magic_close(mcp);
		return NULL;
	}
	tmp_len = strlen(tmp);
	mime_str = malloc(sizeof(*mime_str) * (tmp_len + 1));
	if (mime_str == NULL) {
		magic_close(mcp);
		return NULL;
	}
	strncpy(mime_str, tmp, tmp_len);
	mime_str[tmp_len] = '\0';
	magic_close(mcp);
	return mime_str;
}

/**
 * get_file_size - return the size of a file
 * @f: file to mesure size
 */
static long get_file_size(FILE *f)
{
	long fsize = 0;
	int ret = 0;
	if (f == NULL)
		return -1;
	ret = fseek(f, 0, SEEK_END);
	if (ret < 0)
		return ret;
	fsize = ftell(f);
	if (fsize < 0)
		return fsize;
	ret = fseek(f, 0, SEEK_SET);  //same as rewind(f);
	if (ret < 0)
		return ret;
	return fsize;
}

/**
 * get_file_size_from_path - return file size from pathname
 * @path: path of the file
 * return -1 in case of error
 */
static long get_file_size_from_path(const char *path)
{
	FILE *file = NULL;
	long file_size = 0;
	if (path == NULL)
		return -1;
	file = fopen(path, "r");
	if (file == NULL)
		return -1;
	file_size = get_file_size(file);
	fclose(file);
	return file_size;
}


struct file_data *create_file_data(const char *path)
{
	struct file_data *entry = NULL;
	struct stat st = {0};
	if (stat(path, &st) != 0)
		return NULL;
	entry = malloc(sizeof(*entry));
	if (entry == NULL)
		return NULL;
	entry->path = strdup(path);
	if (entry->path == NULL) {
		free(entry);
		return NULL;
	}
	entry->mime = get_mime_string(path);
	if (entry->mime == NULL) {
		free(entry->path);
		free(entry);
		return NULL;
	}
	entry->data_size = get_file_size_from_path(path);
	if (entry->data_size < 0) {
		free(entry->path);
		free(entry);
		return NULL;
	}
	return entry;
}

void destroy_file_data(struct file_data *entry)
{
	if (entry == NULL)
		return;
	free(entry->path);
	free(entry->mime);
	free(entry);
}

/**
 * get_true_file_path - get true file path from the given url
 * @cp: content_proxy which manages file system information
 * @url: url to a given resource
 */
static char *get_true_file_path(struct content_proxy *cp, char *url, bool from_cache)
{
	char *path = NULL;
	if (from_cache)
		path = string_concat(cp->cache, url);
	else
		path = string_concat(cp->root, url);
	return path;
}

/**
 * get_file_data - retrieve file data
 * @cp: content proxy who holds the information to retrieve the content
 * @url: url to the requested content
 * the content proxy first search in the cache_queue, if the search hasn't
 * success, then retrieve the file from the file system and add it to the cache
 * then return it.
 */
struct file_data *get_file_data(struct content_proxy *cp, char *url, bool from_cache)
{
	char *path = NULL;
	struct file_data *file_data = NULL;
	if (cp == NULL || url == NULL)
		return NULL;
	if (strlen(url) == 1 && *url == '/')
		url = cp->index;
	path = get_true_file_path(cp, url, from_cache);
	if (path == NULL)
		return NULL;
	file_data = create_file_data(path);
	free(path);
	return file_data;
}

struct content_proxy *create_content_proxy(struct content_proxy_settings *cps)
{
	struct content_proxy *cp = NULL;
	int res = 0;
	if (cps == NULL)
		return NULL;
	if (cps->root == NULL || cps->cache == NULL)
		return NULL;
	cp = malloc(sizeof(*cp));
	if (cp == NULL)
		return NULL;
	cp->root = cps->root;
	cp->cache = cps->cache;
	cp->index = cps->index;
	res = pthread_mutex_init(&cp->mutex, NULL);
	if (res < 0) {
		free(cp);
		return NULL;
	}
	return cp;
}

void destroy_content_proxy(struct content_proxy *cp)
{
	if (cp == NULL)
		return;
	pthread_mutex_destroy(&cp->mutex);
	free(cp);
}

static int add_in_cache(struct content_proxy *cp,
				      char *url,
				      char *cache_filename,
				      int weight)
{
	char *entry_path = NULL;
	char *original = NULL;
	int res = 0;
	original = string_concat(cp->root, url);
	if (original == NULL)
		return -1;
	entry_path = string_concat(cp->cache, cache_filename);
	if (entry_path == NULL) {
		free(original);
		return -1;
	}
	res = compress_image(original, entry_path, weight);
	if (res < 0) {
		free(entry_path);
		free(original);
		return -1;
	}
	return 0;
}

static struct file_data *search_in_cache(struct content_proxy *cp, char *url, int weight)
{
	char *cache_filepath = NULL;
	char str_weight[64] = {0};
	struct file_data *content = NULL;
	int res = 0;
	res = snprintf(str_weight, 63, "/q=%d", weight);
	if (res < 0 || res > 63)
		return NULL;
	// *url is '/'
	cache_filepath = string_concat(str_weight, url + 1);
	if (cache_filepath == NULL)
		return NULL;
	content = get_file_data(cp, cache_filepath, true);
	if (content != NULL) {
		free(cache_filepath);
		return content;
	}
	res = add_in_cache(cp, url, cache_filepath, weight);
	if (res < 0) {
		free(cache_filepath);
		return NULL;
	}
	content = get_file_data(cp, cache_filepath, true);
	free(cache_filepath);
	return content;
}

static bool is_image(const struct file_data *file)
{
	return (strstr(file->mime, "image") != NULL);
}

static int parse_accept(char *mime, char *accept)
{
	char *tmp = NULL;
	double dweight = 0.0;
	int weight = 0;
	tmp = strchr(mime, ';');
	if (tmp == NULL) {
		dweight = search_weight_from_mime(accept, mime);
	} else {
		*tmp = '\0';
		dweight = search_weight_from_mime(accept, mime);
		*tmp = ';';
	}
	if (dweight > 0.0 && dweight < 1.0)
		weight = (int)(dweight * 100);
	else
		weight = -1;
	return weight;
}

struct file_data *get_content(struct content_proxy *cp, char *url, char *accept)
{
	struct file_data *content = NULL, *tmp = NULL;
	int weight = 0;
	if (cp == NULL || url == NULL)
		return NULL;
	if (*url != '/')
		return NULL;
	content = get_file_data(cp, url, false);
	if (content == NULL)
		return NULL;
	if (accept != NULL && is_image(content)) {
		weight = parse_accept(content->mime, accept);
		if (weight < 1 || weight > 99) {
			weight = parse_accept("*/*", accept);
			if (weight < 1 || weight > 99)
				return content;
		}
		pthread_mutex_lock(&cp->mutex);
		tmp = search_in_cache(cp, url, weight);
		if (tmp != NULL) {
			destroy_file_data(content);
			content = tmp;
		}
		pthread_mutex_unlock(&cp->mutex);
	}
	return content;
}
