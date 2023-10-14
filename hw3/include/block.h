#ifndef SF_BLOCK_H
#define SF_BLOCK_H

#include "debug.h"

typedef struct sf_sizes {
    size_t HEADER_SIZE;
    size_t FOOTER_SIZE;
    size_t ALIGNMENT_SIZE;
    size_t MIN_BLOCK_SIZE;
    size_t HEADER_WIDTH;
    size_t PAYLOAD_WIDTH;
} sf_sizes;

extern sf_sizes SF_SIZES;


int find_allocation_index(int total_bytes, int min_size);

struct sf_block* find_next_free_block(int total_bytes, size_t payload_size, int min_size);

int free_allocated_block(sf_block* block);

#endif