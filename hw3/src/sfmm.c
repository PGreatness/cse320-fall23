/**
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "debug.h"
#include "sfmm.h"
#include "block.h"

#ifndef SIZES

#define HEADER_SIZE 8
#define FOOTER_SIZE 8
#define ALIGNMENT_SIZE 16
#define MIN_BLOCK_SIZE 32

#endif

void *sf_malloc(size_t size) {
    if (size == 0)
        return NULL;
    int size_to_use = size;
    if (size < 16)
        size_to_use = 16;
    int total_size = size_to_use + HEADER_SIZE + FOOTER_SIZE;
    int added_size = (total_size % ALIGNMENT_SIZE == 0)
                        ? 0
                        : ALIGNMENT_SIZE - (total_size % ALIGNMENT_SIZE);
    total_size += added_size;
    debug("total size: %d, added_size: %d, size before add: %d, padding added: %ld, original size: %zu", total_size, added_size, total_size - added_size, size_to_use - size == 0 ? added_size : size_to_use - size, size);

    struct sf_block* sfb = find_next_free_block(total_size, size, MIN_BLOCK_SIZE);
    if (sfb == NULL)
    {
        debug("sfb is null");
        errno = ENOMEM;
        return NULL;
    }
    return sfb->body.payload;
    // abort();
}

void sf_free(void *pp) {
    // To be implemented.
    sf_block* sfb = (sf_block*) (pp - ALIGNMENT_SIZE);
    if (free_allocated_block(sfb) == -1)
    {
        debug("free_allocated_block returned -1");
        abort();
    }
    return;
    // abort();
}

void *sf_realloc(void *pp, size_t rsize) {
    // To be implemented.
    abort();
}

double sf_fragmentation() {
    // To be implemented.
    abort();
}

double sf_utilization() {
    // To be implemented.
    abort();
}
