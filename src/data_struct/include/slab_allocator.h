#ifndef SLAB_ALLOCATOR_H
#define SLAB_ALLOCATOR_H

#include <stdlib.h>
#include <limits.h>
#include "linked_list.h"

struct slab_allocator {
        size_t size;
        char *slab;
        size_t sizeof_struct;
        int *free_index;
        size_t index_bit_size;
};

struct slab_allocator *create_slab_allocator(size_t n_structs, size_t sizeof_struct);
void destroy_slab_allocator(struct slab_allocator *alloc);
void free_all_struct(struct slab_allocator *alloc);

void *get_struct(struct slab_allocator *alloc);
void free_struct(struct slab_allocator *alloc, void *to_free);


#endif
