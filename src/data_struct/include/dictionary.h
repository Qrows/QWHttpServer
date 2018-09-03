#ifndef STRING_DICT_H
#define STRING_DICT_H

#include "linked_list.h"
#include "slab_allocator.h"
#include <stdlib.h>
#include <string.h>

enum dict_error {
        SD_SUCCESS = 0,
        SD_INVALID_INPUT = -1,
        SD_FAILED_MEM_ALLOCATION = -2,
        SD_KEY_NOT_UNIQUE = -3,
        SD_KEY_NOT_FOUND = -4,
        SD_UNKWOWN_ERROR = -5
};

struct string_dict_elem {
        char *key;
        char *elem;
};


struct string_dict {
        struct linked_list *hashtable;
        size_t hashtable_size;
        size_t hashtable_nelem;
        struct slab_allocator *node_allocator;
        struct slab_allocator *string_dict_elem_allocator;
};


/* DESCRIPTION:
 * create_string_dict() will allocate a new  struct string_dict in the heap.
 * the String_Dictionary will contain max_nelem number of struct at max inside.
 * the char (*hash)(char *) function will be the function that
 * will be used to hash the key inside the string_dictionary.
 * RETURN VALUE:
 * A valid pointer to a string_dict struct in case of success, NULL in case of failure.
 */
struct string_dict *create_string_dict(size_t max_nelem);


/* DESCRIPTION:
 * destroy_string_dict() will free the memory allocated by the create_string_dict() function.
 * it will not free the memory used for the key and the element stored inside.
 */
void destroy_string_dict(struct string_dict *string_dict);

/* DESCRIPTION:
 * string_dict_search() will search for an element stored in a struct string_dict with key equal to *key.
 * Equal is defined by key_cmp and hash function.
 */
char *string_dict_search(struct string_dict *string_dict, char *key);

/* DESCRIPTION:
 * string_dict_add() will add a pair (key, elem) to the string_dictionary pointed by string_dict.
 * string_dict_add() will only copy the pointer key and elem and not the value.
 * if the key is already present inside the string_dictionary, this function
 * return error
 * RETURN VALUE:
 * this function will return 0 in case of success, -1 otherwise.
 */
int string_dict_add(struct string_dict *string_dict, char *key, char *elem);

/* DESCRIPTION:
 * string_dict_delete() will delete a pair (key, elem) from the string_dictionary with key
 * equal to *key, then if not NULL, will store the pointer to the key and elem
 * in key_tofree and elem_tofree pointers if this pointer are not NULL,
 * so that the caller can safely free the two.
 * equal is defined by the hash function and the key_cmp function.
 * RETURN VALUE:
 * this function will return 0 in case of success, -1 otherwise.
 */
int string_dict_delete(struct string_dict *string_dict, char *key);

/* DESCRIPTION:
 * string_dict_reset() will delete all pair (key, elem) in the string_dictionary and free them if
 * if the function pointer free_key and free_elem are not NULL.
 * RETURN VALUE:
 * this function will return 0 in case of success, -1 otherwise.
 */
void string_dict_reset(struct string_dict *string_dict);

#endif
