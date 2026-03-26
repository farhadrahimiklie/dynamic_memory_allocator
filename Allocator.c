#include "Allocator.h" // import the Allocator.h file for using the API's
#include <unistd.h> // used for brk, sbrk for heap memory allocation
#include <sys/mman.h> // for mmap expand the heap storage with mmap using os
#include <string.h> // for string functions
#include <stdint.h> // max_int, min_int

static block_header *heap_head = NULL;
static block_header *bin[BIN_COUNT] = {0}; // BIN_COUNT = 16 (BIN_COUNT -1)

static int get_free_block_index_from_bin(size_t size){
   int index = 0;
   size >>= 4; // devide by 16
   
   while (size && index < BIN_COUNT -1) {
       size >>= 1;
       index++;
   }
   return index;
}

static void bin_insert_free_blocks(block_header *block){
    int index = get_bin_index(block->size);

    block->next_free_block = bin[index];
    block->prev_free_block = NULL;

    if (bin[index] != NULL) {
        bin[index]->prev_free_block = block;
    }
    bin[index] = block;
}

static void bin_remove_free_blocks(block_header *block){
    int index = get_bin_index(block->size);
    if (block->prev_free_block != NULL) {
        block->prev_free_block->next_free_block = block->next_free_block;
    }

    if (block->next_free_block != NULL) {
        block->next_free_block->prev_free_block = block->prev_free_block;
    }

    if (bin[index] == block) {
        bin[index] = block->next_free_block;
    }

    block->next_free_block = NULL;
    block->prev_free_block = NULL;
}

static block_header *find_free_block(size_t size){
    int index = get_bin_index(size); // 100
    for (int i = index; i < BIN_COUNT; i++) {
        block_header *current = bin[i]; // 3

        while (current != NULL) {
            if (current->size >= size) {
                return current;
            }
            current = current->next_free_block;
        }
    }
    return NULL;
}

static void split_unused_byte_of_blocks(block_header *block, size_t size){
    if (block->size <= size + HEADER_SIZE + ALIGNMENT) {
        return;
    }
    block_header *new_block = (block_header*)((char*)(block +1) + size);

    new_block->size = block->size - size - HEADER_SIZE;
    new_block->free = 1; // this block is now free
    new_block->mmapped = 0; // we not use the mmap

    new_block->prev_physical_block = block;
    new_block->next_physical_block = block->next_physical_block;

    if (new_block->next_physical_block != NULL) {
        new_block->next_physical_block->prev_physical_block = new_block;
    }

    new_block->next_free_block = NULL;
    new_block->prev_free_block = NULL;

    block->size = size;
    block->next_physical_block = new_block;

    bin_insert_free_blocks(new_block);
}

static void Coalesce_merge(block_header *block){
    if (block->next_physical_block != NULL && block->next_physical_block->free) {
        block_header *next = block->next_physical_block;
        bin_remove_free_blocks(next);

        block->size += HEADER_SIZE + next->size;
        block->next_physical_block = next->next_physical_block;

        if (block->next_physical_block != NULL) {
            block->next_physical_block->prev_physical_block = block;
        }
    }
    if (block->prev_physical_block != NULL && block->prev_physical_block->free) {
        block_header *prev = block->prev_physical_block;
        bin_remove_free_blocks(prev);

        prev->size += HEADER_SIZE + block->size;
        prev->next_physical_block = block->next_physical_block;

        if (block->next_physical_block != NULL) {
            block->next_physical_block->prev_physical_block = prev;
            block = prev;
        }
    }
    bin_insert_free_blocks(block);
}

static block_header *request_free_space(size_t size){
    void *ptr = sbrk(HEADER_SIZE + size);
    if (ptr == (void*)-1) {
        return NULL;
    }

    block_header *block = (block_header*)ptr;
    block->size = size;
    block->free = 0;
    block->mmapped = 0;

    block->next_physical_block = NULL;
    block->prev_physical_block = NULL;
    block->next_free_block = NULL;
    block->prev_free_block = NULL;

    return block;
}

void *my_malloc(size_t size){
    if (size == 0) {
        return NULL;
    }

    size = ALIGN(size);

    if (size >= MMAP_THREESHOLD) {
        size_t total = HEADER_SIZE + size;
        
        block_header *block = mmap(NULL, total, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        
        if (block == MAP_FAILED) {
            return NULL;
        }

        block->size = size;
        block->free = 0;
        block->mmapped = 1;
        block->next_physical_block = NULL;
        block->prev_physical_block = NULL;
        block->next_free_block = NULL;
        block->prev_free_block = NULL;

        return (void*)(block +1);
    }

    block_header *block = find_free_block(size);
    if (block != NULL) {
        bin_remove_free_blocks(block);
        block->free = 0;
        split_unused_byte_of_blocks(block, size);
        return (void*)(block +1);
    }

    block = request_free_space(size);
    if (block == NULL) {
        return NULL;
    }

    if (heap_head == NULL) {
        heap_head = block;
    }else{
        block_header *last = heap_head;
        while (last->next_physical_block != NULL) {
            last = last->next_physical_block;
        }
        last->next_physical_block = block;
        block->prev_physical_block = last;
    }
    return (void*)(block +1);
}

void *my_calloc(size_t n_memeber, size_t size){
    if (size != 0 && n_memeber > SIZE_MAX / size) {
        return NULL;
    }

    size_t total = n_memeber * size;
    void *ptr = my_malloc(total);

    if (ptr != NULL) {
        memset(ptr, 0, total);
    }
    return ptr;
}

void *my_realloc(void *ptr, size_t size){
    if (ptr == NULL) {
        return my_malloc(size);
    }
    if (ptr == 0) {
        my_free(ptr);
        return NULL;
    }
    size = ALIGN(size);

    block_header *block = (block_header*)ptr -1;

    if (block->size >= size) {
        return ptr;
    }

    void *new_ptr = my_malloc(size);

    if (new_ptr == NULL) {
        return NULL;
    }
    memcpy(new_ptr, ptr, block->size);
    my_free(ptr);
    return new_ptr;
}

void my_free(void *ptr){
    if (ptr == NULL) {
        return;
    }

    block_header *block = (block_header*)ptr -1;

    if (block->mmapped) {
        munmap(block, HEADER_SIZE + block->size);
        return;
    }

    block->free = 1;
    Coalesce_merge(block);
}
