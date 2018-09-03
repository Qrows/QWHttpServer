#include "dictionary.h"


static struct node *search_node_by_key(struct linked_list *list,
                                       char *key);
static struct string_dict_elem *search_delem_by_key(struct linked_list *list,
                                                    char *key);


struct string_dict *create_string_dict(size_t max_nelem)
{
        /* create a new string_dict struct */
        struct string_dict *new;
        new = malloc(sizeof(*new));
        if (new == NULL)
                return NULL;
        new->node_allocator = create_slab_allocator(max_nelem, sizeof(struct node));
        if (new->node_allocator == NULL) {
                free(new);
                return NULL;
        }
       new->string_dict_elem_allocator = create_slab_allocator(max_nelem, sizeof(struct string_dict_elem));
        if (new->string_dict_elem_allocator == NULL) {
                destroy_slab_allocator(new->node_allocator);
                free(new);
                return NULL;
        }
        /* the hash table size is twice the number of element for speed reason.
         * a hash table perform better when is not full. (less collision)
         */
        new->hashtable_size = 2*max_nelem;
        new->hashtable = calloc(new->hashtable_size, sizeof(*new->hashtable));
        if (new->hashtable == NULL) {
                destroy_slab_allocator(new->node_allocator);
                destroy_slab_allocator(new->string_dict_elem_allocator);
                free(new);
                return NULL;
        }
        for (size_t i = 0; i < new->hashtable_size; i++)
                init_linked_list(&new->hashtable[i]);
        new->hashtable_nelem = 0;
        return new;
}

static size_t fhash(char *str)
{
        /* djb2 hash algoritm */
        /* magic number */
        size_t hash = 5381;
        while (*str != '\0') {
                // hash = (hash * 33) xor key_buf[i]
                hash = (hash + (hash << 5)) ^ ((size_t)*str);
                ++str;
        }
        return hash;

}


void destroy_string_dict(struct string_dict *string_dict)
{
        /* free a string_dict struct*/
        if (string_dict == NULL)
                return;
        destroy_slab_allocator(string_dict->node_allocator);
        destroy_slab_allocator(string_dict->string_dict_elem_allocator);
        free(string_dict->hashtable);
        free(string_dict);
}


char *string_dict_search(struct string_dict *string_dict, char *key)
{
        /* search for the elem with key equal to *key */
        size_t hash;
        struct linked_list *hash_list;
        struct string_dict_elem *d_elem;
        if (string_dict == NULL)
                return NULL;
        /* calculate hash and position inside the string_dictionary */
        hash = fhash(key) % string_dict->hashtable_size;
        hash_list = &string_dict->hashtable[hash];
        /* check inside the linked list if the key is found,
         * there could be multiple pairs cause of collision
         */
        d_elem = search_delem_by_key(hash_list, key);
        if (d_elem == NULL)
                return NULL;
        return d_elem->elem;

}



void string_dict_reset(struct string_dict *string_dict)
{
        size_t i;
        for (i = 0; i < string_dict->hashtable_size; i++) {
                struct linked_list *list = &string_dict->hashtable[i];
                while (!linked_list_is_empty(list)) {
                        struct node *node;
                        struct string_dict_elem *d_elem;
                        node = linked_list_pop(list);
                        /* this check should never happen, but better safe than sorry */
                        if (node == NULL)
                                break;
                        d_elem = node->data;
                        free(d_elem->key);
                        free(d_elem->elem);
                        free_struct(string_dict->string_dict_elem_allocator, (char *)d_elem);
                        free_struct(string_dict->node_allocator, (char *)node);
                }
        }
        string_dict->hashtable_nelem = 0;
}

static int is_key_inside_key_list(struct linked_list *key_list,
                                  char *key)
{
        /* return true(not 0) if key is alreadey present inside a
         * linked list of string_dict_elem key_list, return false(0) otherwise
         */
        struct node *curr;
        curr = key_list->start;
        while (curr != NULL) {
                struct string_dict_elem *d_elem;
                d_elem = (struct string_dict_elem *)curr->data;
                if (strcmp(key, d_elem->key) == 0)
                        return 1;
                curr = curr->next;
        }
        return 0;
}


static struct node *get_new_dnode(struct string_dict *string_dict, char *key, char *elem)
{
        /* create a new node with a string_dict_elem as data for *string_dict */
        struct node *node;
        struct string_dict_elem *string_dict_elem;
        node = get_struct(string_dict->node_allocator);
        if (node == NULL)
                return NULL;
        string_dict_elem = get_struct(string_dict->string_dict_elem_allocator);
        if (string_dict_elem == NULL) {
                /* this should never happen, but better safe than sorry */
                free_struct(string_dict->node_allocator, node);
                return NULL;
        }
        /* copy the key and value pointer inside a string_dict_elem */
        string_dict_elem->key = key;
        string_dict_elem->elem = elem;
        node->data = string_dict_elem;
        return node;

}

static char *alloc_and_copy(char *str)
{
        char *copy;
        size_t str_len;
        if (str == NULL)
                return NULL;
        str_len = strlen(str);
        copy = malloc(sizeof(*copy) * str_len);
        if (copy == NULL)
                return NULL;
        strncpy(copy, str, str_len);
        return copy;
}


int string_dict_add(struct string_dict *string_dict, char *key, char *elem)
{
        /* add a new pair (key, elem) inside the string_dictionary *string_dict */
        struct node *node;
        size_t hash;
        char *dkey, *delem;
        if (string_dict == NULL || key == NULL || elem == NULL)
                return SD_INVALID_INPUT;
        /* copy key and elem inside dkey and delem so the dict has full controll over
         * the allocation of the string it stores */
        dkey = alloc_and_copy(key);
        if (dkey == NULL)
                return SD_FAILED_MEM_ALLOCATION;
        delem = alloc_and_copy(elem);
        if (delem == NULL) {
                free(dkey);
                return SD_FAILED_MEM_ALLOCATION;
        }
        /* allocate the node and the string_dict_elem*/
        node = get_new_dnode(string_dict, dkey, delem);
        if (node == NULL) {
                free(dkey);
                free(delem);
                return SD_FAILED_MEM_ALLOCATION;
        }
        /* hash and insert in the correct linked list */
        hash = fhash(key) % string_dict->hashtable_size;
        /* if the key already exist inside the linked list return error */
        if (is_key_inside_key_list(&string_dict->hashtable[hash], key)) {
                free_struct(string_dict->string_dict_elem_allocator, (struct string_dict_elem *)node->data);
                free_struct(string_dict->node_allocator, node);
                free(dkey);
                free(delem);
                return SD_KEY_NOT_UNIQUE;
        }
        linked_list_add(&string_dict->hashtable[hash], node);
        string_dict->hashtable_nelem++;
        return SD_SUCCESS;
}

int string_dict_delete(struct string_dict *string_dict, char *key)
{
        /* delete the pair (*key, elem) from the string_dictionary.
         */
        size_t hash;
        struct linked_list *hash_list;
        struct node *to_delete;
        struct string_dict_elem *string_dict_elem;
        int err;
        if (string_dict == NULL)
                return SD_INVALID_INPUT;
        /* hash the key and find the position inside the string_dict */
        hash = fhash(key) % string_dict->hashtable_size;
        hash_list = &string_dict->hashtable[hash];
        /* search inside the linked list in case there are collision */
        to_delete = search_node_by_key(hash_list, key);
        if (to_delete == NULL)
                return SD_KEY_NOT_FOUND;
        string_dict_elem = to_delete->data;
        /* if the key is found remove it from the string_dict */
        err = linked_list_remove(hash_list, to_delete);
        /* this should never fail, but better safe than sorry*/
        if (err < 0)
                return SD_UNKWOWN_ERROR;
        /* free the string, the string_dict_elem and the node*/
        free(string_dict_elem->key);
        free(string_dict_elem->elem);
        free_struct(string_dict->string_dict_elem_allocator, string_dict_elem);
        free_struct(string_dict->node_allocator, to_delete);
        /* number of element in the dictionary decremented by one*/
        string_dict->hashtable_nelem--;
        return SD_SUCCESS;
}

static struct node *search_node_by_key(struct linked_list *list,
                                       char *key)
{
        struct node *curr;
        struct string_dict_elem *curr_data;
        int cmp;
        for (curr = list->start; curr != NULL; curr = curr->next) {
                curr_data = curr->data;
                cmp = strcmp(key, curr_data->key);
                if (cmp == 0)
                        return curr;
        }
        return NULL;
}

static struct string_dict_elem *search_delem_by_key(struct linked_list *list,
                                                    char *key)
{
        struct node *node;
        node = search_node_by_key(list, key);
        if (node == NULL)
                return NULL;
        return node->data;
}
