#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file_system.h"

void print_file(struct file_data *fd)
{
	size_t i = 0;
	for (i = 0; i < fd->data_size; ++i)
		putchar(fd->data[i]);
	putchar('\n');
}

void print_argv(int argc, char **argv)
{
	printf("%s: %d\n", "argc", argc);
	for (int i = 0; i < argc; ++i) {
		printf("%s", *(argv + i));
		putchar(' ');
	}
	putchar('\n');
	
}

void newline_to_zero(char *s)
{
	char *newline = NULL;
	newline = strchr(s, '\n');
	if (newline == NULL)
		return;
	else
		*newline = '\0';
	return;
}

int main(int argc, char **argv)
{
	struct content_proxy *cp = NULL;
	int err = 0;
	char *line = NULL;
	size_t line_len = 0;
	struct file_data *file_data = NULL;
	if (argc < 2) {
		fprintf(stderr, "%s root\n", *argv);
		return EXIT_FAILURE;
	}
	print_argv(argc, argv);
	cp = create_content_proxy(argv[1], 3);
	if (cp == NULL) {
		fprintf(stderr, "%s\n", "create_content_proxy: failed!");
		return EXIT_FAILURE;
	}
	while ((err = getline(&line, &line_len, stdin)) != -1) {
		newline_to_zero(line);
		printf("%s:%s\n", "line", line);
		file_data = get_file_data(cp, line);
		if (file_data == NULL) {
			fprintf(stderr, "%s\n", "get_file_data() failed!");
			free(line);
			line = NULL;
			line_len = 0;
			continue;
		}
		print_cache(cp);
		free(line);
		line = NULL;
		line_len = 0;
	}
	return EXIT_SUCCESS;
}
