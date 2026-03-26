#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>

#define ALIGNMENT 8
#define ALIGN(x) (((x) + (ALIGNMENT -1)) & ~(ALIGNMENT -1))

#define MMAP_THREESHOLD 131072
#define BIN_COUNT 16

// struct of our Blocks

typedef struct block_header{
    size_t size;
    int free;
    int mmapped; // 1 for true, 0 for false

    struct block_header *next_physical_block; // track the free next and prev block insdie the blocks
    struct block_header *prev_physical_block;

    struct block_header *next_free_block; // track the free next and prev free space inside the bins array
    struct block_header *prev_free_block; // track the free next and prev free space insdie the bins array
}block_header;

#define HEADER_SIZE sizeof(block_header) // size of the header structure is 48
                                         
void *my_malloc(size_t size); // initialize with garbage values
void *my_calloc(size_t n_member, size_t size); // initialize with zero
void *my_realloc(void *ptr, size_t size); // first get the address of prev block and then expand it with new size
void my_free(void *ptr); // now free the block and return it for OS.
#endif
