#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "string_utils.h"

int main(void)
{
	char *str = NULL;
	size_t count = 0;
	char **strs = NULL;
	str = string_concat("Hello", " World");
	if (str == NULL) {
		fprintf(stderr, "%s\n", "concat() failed!");
		return EXIT_FAILURE;
	}
	printf("%s\n", str);
	count = string_count_char_occurrence(str, 'l');
	printf("l: %zu\n", count);
	strs = string_split_char_token(str, 'l');
		if (strs == NULL) {
		fprintf(stderr, "%s\n","split_char() failed!");
		return EXIT_FAILURE;
	}
	printf("%s\n", str);
	for (size_t i = 0; strs[i] != NULL; ++i) {
		printf("strs[%zu] = %s\n", i, strs[i]);
	}
	string_free_string_list(strs);
	strs = string_split_substring_token(str, "ll");
	if (strs == NULL) {
		fprintf(stderr, "%s\n","split_substring() failed!");
		return EXIT_FAILURE;
	}
	printf("%s\n", str);
	for (size_t i = 0; strs[i] != NULL; ++i) {
		printf("strs[%zu] = %s\n", i, strs[i]);
	}
	free(str);
	string_free_string_list(strs);
	return EXIT_SUCCESS;
	
}
