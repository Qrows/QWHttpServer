#include "linked_list.h"


struct linked_list *create_linked_list(void)
{
        /* create a new linked list struct in the heap and init them*/
        struct linked_list *res;
        res = malloc(sizeof(*res));
        if (res == NULL)
                return NULL;
        init_linked_list(res);
        return res;
}
void init_linked_list(struct linked_list *list)
{
        /* init a linked list struct */
        if (list == NULL)
                return;
        list->start = NULL;
        list->end = &list->start;
}

void deinit_linked_list(struct linked_list *list, void (*free_data)(void *))
{
        struct node *curr;
        if (list == NULL || free_data == NULL)
                return;
        while (!linked_list_is_empty(list)) {
                curr = linked_list_pop(list);
                destroy_node(curr, free_data);
        }
}

void destroy_linked_list(struct linked_list *list, void (*free_data)(void *))
{
        if (list == NULL || free_data == NULL)
                return;
        deinit_linked_list(list, free_data);
        free(list);
}

struct node *linked_list_get_start(struct linked_list *list)
{
        return list->start;
}

struct node *create_node(void *data)
{
        struct node *res;
        res = malloc(sizeof(*res));
        if (res == NULL)
                return NULL;
        init_node(res, data);
        return res;
}

void init_node(struct node *node, void *data)
{
        if (node == NULL)
                return;
        node->data = data;
        node->next = NULL;
}


void deinit_node(struct node *node, void (*free_data)(void *))
{
        if (node == NULL || free_data == NULL)
                return;
        free_data(node->data);
        node->next = NULL;
}


void destroy_node(struct node *node, void (*free_data)(void *))
{
        if (node == NULL || free_data == NULL)
                return;
        deinit_node(node, free_data);
        free(node);
}

int linked_list_is_empty(struct linked_list *list)
{
        if (list == NULL)
                return 1;
        return list->start == NULL;
}

void linked_list_add(struct linked_list *list, struct node *new_node)
{
        if (list == NULL || new_node == NULL)
                return;
        *(list->end) = new_node;
        list->end = &(new_node->next);
}

struct node *linked_list_pop(struct linked_list *list)
{
        struct node *ret;
        if (list == NULL)
                return NULL;
        if (list->start == NULL)
                return NULL;
        /* if in the list remain only one node element */
        if (list->start->next == *list->end) {
                ret = list->start;
                /* here list become empty, reset pointer state */
                init_linked_list(list);
        } else {
                ret = list->start;
                list->start = list->start->next;
        }
        ret->next = NULL;
        return ret;
}

int linked_list_remove(struct linked_list *list, struct node *node)
{
        struct node **next;
        if (list == NULL || node == NULL)
                return -1;
        if (list->start == NULL)
                return -1;
        next = &list->start;
        while (*next != NULL) {
                struct node *curr = *next;
                if (curr == node) {
                        *next = curr->next;
                        node->next = NULL;
                        return 0;
                }
                next = &(curr->next);
        }
        return -1;
}
