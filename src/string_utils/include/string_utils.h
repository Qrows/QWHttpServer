#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

char *string_concat(const char *s1, const char *s2);
/* string_concat() will concat the string s1 and s2
 * in a new malloc(2) allocated string.
 * RETURN:
 * This function return NULL in case of failure, or a
 * valid pointer to the new string otherwise.
 */

char *string_trim(const char *s1, char ch);
/* string_trim() will trim all occurrence of consecutive ch 
 * from the start and the end of s and save it in a new string.
 */
size_t string_count_char_occurrence(const char *s, char ch);
/* string_count_char_occurrence() will count che number of occurence
 * of ch in s.
 * RETURN:
 * return the number of occurence counted, in case of error set errno
 * and return 0.
 */
char *extract_string_from_token(const char *s, char tok, size_t *tok_index);
/* extract_string_from_token() will search for the first tok occurrence inside
 * s and return a new allocated string that end at the char before tok.
 * RETURN:
 * in case of success return the string extracted and set *tok_index to
 * the index where tok was found, otherwise return NULL.
*/
size_t string_count_substring_unique_occurrence(const char *s,
						const char *substring);
/* string_count_substring_unique_occurrence() will count all the unique
 * repetition of substring inside s, unique means that overlap by substrings
 * inside s are not allowed (example: s = "ababa" substring "aba") and
 * only the first will be count (in the example only the first "aba" is counted
 * the second is consider only as "ba").
 * RETURN:
 * return the number of unique substring found in case of success,
 * in case of failure set errno and return 0.
 */
char **string_split_char_token(const char *s, char tok);
/* string_split_char_token() will split the string s in multiple
 * string separated by token and return it in a null terminated
 * array of string, all the string in the array plus the array itself
 * are malloc(2) allocated.
 * RETURN:
 * return NULL in case of failure, otherwise return a valid pointer to
 * a NULL terminated array of string containing the splitted string.
 */
char **string_n_split_char_token(const char *s, char tok, size_t n);
/* string_split_char_token() will split the string s in n
 * string separated by token and return it in a null terminated
 * array of string, all the string in the array plus the array itself
 * are malloc(2) allocated.
 * RETURN:
 * return NULL in case of failure, otherwise return a valid pointer to
 * a NULL terminated array of string containing the splitted string.
 */ 
char **string_split_substring_token(const char *s,
				    const char *substring);
/* string_split_substring_token() will split the string s in multiple
 * string separated by substring and return it in a null terminated
 * array of string, all the string in the array plus the array itself
 * are malloc(2) allocated.
 * RETURN:
 * return NULL in case of failure, otherwise return a valid pointer to
 * a NULL terminated array of string containing the splitted string.
 */

size_t string_count_string_list_size(char **s);
/* string_count_string_list_size() will count che number of element 
 * of a NULL terminated string array.
 * RETURN:
 * return the number of element counted, in case of error set errno
 * and return 0.
 */

void string_free_string_list(char **s);
/* string_free_string_list() will free NULL terminated array
 * of string all allocated by malloc(2).
 */

char **string_list_concat(char **sl1, char **sl2);
/* string_list_concat() will concat two NULL terminated string 
 *  of string.
 * RETURN:
 * return the result list in case of success or NULL otherwise.
 */


#endif
