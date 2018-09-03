#include "slab_allocator.h"

static size_t get_int_bit();
static void set_bit(int *bitmap, size_t k);
static void clear_bit(int *bitmap, size_t k);
static int test_bit(int *bitmap, size_t k);

struct slab_allocator *create_slab_allocator(size_t n_structs, size_t sizeof_struct)
{
        /* create a slab allocator with n_structs to allocate*/
        size_t int_size;
        struct slab_allocator *new;
        new = malloc(sizeof(*new));
        if (new == NULL)
                return NULL;
        new->slab = calloc(n_structs, sizeof_struct);
        if (new->slab == NULL) {
                free(new);
                return NULL;
        }
        int_size = get_int_bit();
        new->index_bit_size = n_structs/int_size + 1;
        new->free_index = calloc(new->index_bit_size, sizeof(*new->free_index));
        new->size = n_structs;
        new->sizeof_struct = sizeof_struct;
        return new;
}


void destroy_slab_allocator(struct slab_allocator *alloc)
{
        if (alloc == NULL)
                return;
        /* free a slab_allocator created by create_slab_allocator() */
        free(alloc->slab);
        free(alloc->free_index);
        free(alloc);
}


void free_all_struct(struct slab_allocator *alloc)
{
        if (alloc == NULL)
                return;
        for (size_t i = 0; i < alloc->size; i++)
                clear_bit(alloc->free_index, i);
}

void *get_struct(struct slab_allocator *alloc)
{
        /* return the first free slab found or NULL is none found */
        char *res;
        if (alloc == NULL)
                return NULL;
        for (size_t i = 0; i < alloc->size; i++) {
                /* if the i-th bit is 0, then the slab is free */
                if (!test_bit(alloc->free_index, i)) {
                        /* set the i-th bit to 1 to say is used */
                        set_bit(alloc->free_index, i);
                        res = alloc->slab + (i * alloc->sizeof_struct);
                        return res;
                }
        }
        return NULL;
}

void free_struct(struct slab_allocator *alloc, void *to_free)
{
        /* free slab, set the state of slab from used to free */
        size_t slab_index;
        if (alloc == NULL || to_free == NULL)
                return;
        /* calculate in which position(index) slab is inside alloc->slab*/
        slab_index = (char *)to_free - alloc->slab;
        /* set the slab from used to free */
        clear_bit(alloc->free_index, slab_index);
}

static size_t get_int_bit()
{
        /* count the number of bits inside a int type */
        int x;
        size_t counter;
        x = INT_MAX;
        counter = 0;
        /* x is initialized as the greatest supported integer on the system,
         * so all his bit are 1. we shift to right his bit by one position
         * (we divide by 2) until x become 0.
         * when x become 0 x >>= 1 return 0 and we exit the while loop.
         */
        while (x >>= 1)
                counter++;
        return counter;
}

static void set_bit(int *bitmap, size_t k)
{
        /* set the k-th bit inside *bitmap to 1
         * bitmap[k/sizeof(*bitmap)] = in which int we found the k-th bit.
         * (1 << (k % sizeof(*bitmap))) = all 0 except the k-th bit that is 1.
         */
        bitmap[k/sizeof(*bitmap)] |= (1 << (k % sizeof(*bitmap)));
}


static void clear_bit(int *bitmap, size_t k)
{
        /* set the k-th bit inside *bitmap to 0
         * bitmap[k/sizeof(*bitmap)] = in which int we found the k-th bit.
         * ~(1 << (k % sizeof(*bitmap))) = all 1 except the k-th bit that is 0.
         */
        bitmap[k/sizeof(*bitmap)] &= ~(1 << (k % sizeof(*bitmap)));
}


static int test_bit(int *bitmap, size_t k)
{
        /* return true if the value of the k-th bit is 1, false otherwise.
         * * bitmap[k/sizeof(*bitmap)] = in which int we found the k-th bit.
         * (1 << (k % sizeof(*bitmap))) = all 0 except the k-th bit that is 1.
         * the & from these two return 0 if the k-th bit is 0, != 0 otherwise.
         */
        return (bitmap[k/sizeof(*bitmap)] & (1 << (k % sizeof(*bitmap)))) != 0;
}
