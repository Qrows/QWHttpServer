#include "string_utils.h"

char *string_concat(const char *s1, const char *s2)
{
	/* concat s1 and s2 and put the result in a newly allocated string*/
	char *res = NULL;
	size_t size = 0;
	if (s1 == NULL || s2 == NULL) {
		errno = EINVAL;
		return NULL;
	}
	size = strlen(s1) + strlen(s2) + 1;
	res = malloc(size);
	if (res == NULL)
		return NULL;
	*res = '\0';
	strcpy(res, s1);
	strcat(res, s2);
	res[size - 1] = '\0';
	return res;
}

size_t string_count_char_occurrence(const char *s, char ch)
{
	/* count all occurrence of ch inside s*/
	size_t occur = 0;
	char *ptr = NULL;
	if (s == NULL) {
		errno = EINVAL;
		return 0;
	}
	while ((ptr = strchr(s, ch)) != NULL) {
		s = ptr + 1;
		++occur;
	}
	return occur;
}

char *string_trim(const char *s, char ch)
{
	/* trim ch character from the start and end of s
	 * example ch = ' ' s = "   string   " -> "string"
	 */
	size_t trim_start = 0;
	size_t trim_end = 0;
	size_t i = 0;
	size_t slen = 0;
	char *res = NULL;
	if (s == NULL) {
		errno = EINVAL;
		return NULL;
	}
	// count the occurence of ch from start
	// when found different char break
	trim_start = 0;
	for (i = 0; s[i] != '\0'; ++i) {
		if (s[i] != ch)
			break;
		trim_start++;
	}
	// count from the start of the string
	// if s[i] is not char reset counter
	// because it must be a contiguos block
	// of char till \0
	trim_end = 0;
	for (i = 0; s[i] != '\0'; i++) {
		if (s[i] != ch) {
			trim_end = 0;
		}
		trim_end++;
	}
	// copy result in new string
	slen = strlen(s + trim_start);
	res = strndup(s + trim_start, slen - trim_end + 1);
	return res;
}

char *extract_string_from_token(const char *s, char tok, size_t *tok_index)
{
	/* return the string that go to s to the first occurence of tok*/
	char *res = NULL;
	char *ptr = NULL;
	if (s == NULL) {
		errno = EINVAL;
		return NULL;
	}
	ptr = strchr(s, tok);
	if (ptr == NULL)
		return NULL;
	res = strndup(s, ptr - s);
	if (tok_index)
		*tok_index = (ptr - s);
	return res;
}

char **string_split_char_token(const char *s, char tok)
{
	/* split s by tok*/
	char **res = NULL;
	size_t res_size = 0;
	size_t i = 0;
	size_t curr = 0;
	// all token occurence plus the last string and the NULL terminating
	res_size = string_count_char_occurrence(s, tok) + 2;
	res = malloc(sizeof(*res) * (res_size));
	if (res == NULL)
		return NULL;
	curr = 0;
	for (i = 0; i < res_size - 2; ++i) {
		size_t tmp = 0;
		res[i] = extract_string_from_token(s + curr, tok, &tmp);
		if (res[i] == NULL) {
			string_free_string_list(res);
			return NULL;
		}
		curr += tmp + 1;
	}
	// handle last cases which end in \0 instead of tok
	res[res_size - 2] = strdup(s + curr);
	if (res[res_size - 2] == NULL) {
		string_free_string_list(res);
		return NULL;
	}
	res[res_size - 1] = NULL;
	return res;
}

char **string_n_split_char_token(const char *s, char tok, size_t n)
{
	/* split s by tok*/
	char **res = NULL;
	size_t res_size = 0;
	size_t i = 0;
	size_t curr = 0;
	size_t n_occur = 0;
	// all token occurence plus the last string and the NULL terminating
	n_occur = string_count_char_occurrence(s, tok);
	res_size = (n < n_occur) ? (n + 1) : (n_occur + 2);
	res = malloc(sizeof(*res) * (res_size));
	if (res == NULL)
		return NULL;
	curr = 0;
	for (i = 0; i < res_size - 2; ++i) {
		size_t tmp = 0;
		res[i] = extract_string_from_token(s + curr, tok, &tmp);
		if (res[i] == NULL) {
			string_free_string_list(res);
			return NULL;
		}
		curr += tmp + 1;
	}
	// handle last cases which end in \0 instead of tok
	res[res_size - 2] = strdup(s + curr);
	if (res[res_size - 2] == NULL) {
		string_free_string_list(res);
		return NULL;
	}
	res[res_size - 1] = NULL;
	return res;
}

size_t string_count_substring_unique_occurrence(const char *s,
						const char *substring)
{
	/* count all the unique substring inside s
	* unique means that overlap by substring inside
	* s are not allowed and only the first will be
	* counted.
	*/
	size_t res = 0;
	char *ptr = NULL;
	size_t sblen = 0;
	if (s == NULL || substring == NULL)
		return 0;
	sblen = strlen(substring);
	while ((ptr = strstr(s, substring)) != NULL) {
		s = ptr + sblen;
		++res;
	}
	return res;
}

char **string_split_substring_token(const char *s,
				    const char *substring)
{
	/* split s by substring and return the splitted string*/
	char **res = NULL;
	size_t res_len = 0;
	char *ptr = NULL;
	size_t i = 0;
	size_t substring_len = 0;
	if (s == NULL || substring == NULL)
		return NULL;
	res_len = string_count_substring_unique_occurrence(s, substring) + 2;
	res = malloc(sizeof(*res) * (res_len));
	if (res == NULL)
		return NULL;
	i = 0;
	substring_len = strlen(substring);
	/* for each substring found copy till it and skip it*/
	for (i = 0; i < res_len - 2 && (ptr = strstr(s, substring)) != NULL; ++i) {
		size_t n = (ptr - s); // index of token
		res[i] = strndup(s, n); // copy till token (not included)
		if (res[i] == NULL) {
			string_free_string_list(res);
			return NULL;
		}
		s = ptr + substring_len;
	}
	res[res_len - 2] = strdup(s);
	res[res_len - 1] = NULL;
	return res;
}


	
size_t string_count_string_list_size(char **s)
{
	size_t count = 0;
	if (s == NULL) {
		errno = EINVAL;
		return 0;
	}
	count = 0;
	while (*s != NULL) {
		++s;
		count++;
	}
	return count;
}

void string_free_string_list(char **s)
{
	/* free a NULL terminated array of malloc(2) allocated string
	* plus the array itself
	*/
	char **tmp = NULL;
	if (s == NULL)
		return;
	tmp = s;
	while (*s != NULL) {
		free(*s);
		++s;
	}
	free(tmp);
}

char **string_list_concat(char **sl1, char **sl2)
{
	char **result = NULL;
	size_t result_size = 0;
	size_t sl1_size = 0;
	size_t sl2_size = 0;
	if (sl1 == NULL || sl2 == NULL)
		return NULL;
	sl1_size = string_count_string_list_size(sl1);
	sl2_size = string_count_string_list_size(sl2);
	result_size = sl1_size + sl2_size + 1;
	result = malloc(sizeof(*result) * (result_size));
	if (result == NULL)
		return NULL;
	memcpy(result, sl1, sl1_size);
	memcpy(result + sl1_size, sl2, sl2_size);
	return result;
}
