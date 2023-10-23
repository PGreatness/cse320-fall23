#ifndef HW3_SFMM_BLOCK_H
#define HW3_SFMM_BLOCK_H

#ifndef SIZES
    #define SFMM_HEADER_SIZE sizeof(sf_header)
    #define SFMM_FOOTER_SIZE sizeof(sf_footer)
    #define SFMM_ALIGNMENT_SIZE (sizeof(sf_header) + sizeof(sf_footer))
    #define SFMM_MIN_BLOCK_SIZE sizeof(sf_block)
    #define SFMM_HEADER_WIDTH 64
    #define SFMM_PAYLOAD_WIDTH 32
#endif

#include "debug.h"
#include "sfmm.h"
#include "sfmm_analytics.h"
typedef struct sfmm_sizes {
    size_t HEADER_SIZE;
    size_t FOOTER_SIZE;
    size_t ALIGNMENT_SIZE;
    size_t MIN_BLOCK_SIZE;
    size_t HEADER_WIDTH;
    size_t PAYLOAD_WIDTH;
} sfmm_sizes;

extern sfmm_sizes SFMM_SIZES;

int find_allocation_index(size_t total_bytes);

struct sf_block* find_next_free_block(size_t total_bytes, size_t payload_size);

int free_allocated_block(sf_block* block);

int can_realloc_without_splinter(sf_block* block, size_t size);

sf_block* realloc_block(sf_block* sfb, size_t new_block_size, size_t new_payload_size);

size_t peek_block_size(sf_block* sfb);
size_t peek_payload_size(sf_block* sfb);
int is_allocated(sf_block* sfb);
sf_block* update_payload_size(sf_block* sfb, size_t update);

double get_peak_utilization();
#endif //HW3_BLOCK_H