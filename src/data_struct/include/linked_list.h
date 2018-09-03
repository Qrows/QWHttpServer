#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdlib.h>

struct node {
        void *data;
        struct node *next;
};

struct linked_list {
        struct node *start;
        struct node **end;
};

struct linked_list *create_linked_list(void);
void destroy_linked_list(struct linked_list *list, void (*free_data)(void *));
void init_linked_list(struct linked_list *list);
void deinit_linked_list(struct linked_list *list, void (*free_data)(void *));
struct node *create_node(void *data);
void destroy_node(struct node *node, void (*free_data)(void *));
void init_node(struct node *node, void *data);
void deinit_node(struct node *node, void (*free_data)(void *));
void linked_list_add(struct linked_list *list, struct node *new_node);
struct node *linked_list_pop(struct linked_list *list);
int linked_list_remove(struct linked_list *list, struct node *node);
int linked_list_is_empty(struct linked_list *list);
struct node *linked_list_get_start(struct linked_list *list);

#endif
