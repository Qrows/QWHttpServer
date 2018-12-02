#include <stdio.h>
#include <stdlib.h>

#include "list.h"

struct mylist {
	char *data;
	struct list_head list;
};

struct mylist *get_mylist(char *data)
{
	struct mylist *new = NULL;
	new = malloc(sizeof(*new));
	if (new == NULL)
		return NULL;
	new->data = data;
	INIT_LIST_HEAD(&new->list);
	return new;
}

int main(int argc, char **argv)
{
	struct mylist *tmp = NULL;
	struct mylist *head = NULL;
	head = get_mylist(argv[0]);
	if (head == NULL) {
		fprintf(stderr, "failed allocation at 0");
		return EXIT_FAILURE;
	}
	for (int i = 0; i < argc; ++i) {
		tmp = get_mylist(argv[i]);
		if (tmp == NULL) {
			fprintf(stderr, "failed allocation at %d-th iteration\n", i);
			return EXIT_FAILURE;
		}
		list_add(&tmp->list, &head->list);
	}
	list_for_each_entry(tmp, &head->list, list) {
		printf("data: %s\n", tmp->data);
	}
	list_for_each_entry(tmp, &head->list, list) {
		list_del(&tmp->list);
		free(tmp);
		list_for_each_entry(tmp, &head->list, list) {
			printf("data: %s\n", tmp->data);
		}
	}
	printf("end\n");
	list_for_each_entry(tmp, &head->list, list) {
		printf("data: %s\n", tmp->data);
	}
	for (int i = 0; i < argc; ++i) {
		tmp = get_mylist(argv[i]);
		if (tmp == NULL) {
			fprintf(stderr, "failed allocation at %d-th iteration\n", i);
			return EXIT_FAILURE;
		}
		list_add(&tmp->list, &head->list);
	}
	list_for_each_entry(tmp, &head->list, list) {
		printf("data: %s\n", tmp->data);
	}
	list_for_each_entry(tmp, &head->list, list) {
		list_del(&tmp->list);
		free(tmp);
		list_for_each_entry(tmp, &head->list, list) {
			printf("data: %s\n", tmp->data);
		}
	}
	printf("end\n");
	return EXIT_SUCCESS; 
}
