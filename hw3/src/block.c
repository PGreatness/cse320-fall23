#include "sfmm.h"
#include "block.h"

sf_sizes SF_SIZES = {
    .HEADER_SIZE = 8,
    .FOOTER_SIZE = 8,
    .ALIGNMENT_SIZE = 16,
    .MIN_BLOCK_SIZE = 32,
    .HEADER_WIDTH = 64,
    .PAYLOAD_WIDTH = 32
};

struct sf_block* find_previous(sf_block* block)
{
    if (block == NULL)
        return NULL;
    int index = 0;

    while (index < NUM_FREE_LISTS - 1)
    {
        sf_block* tmp = &sf_free_list_heads[index];
        while (tmp && (tmp->body.links.next) && (tmp->body.links.next != NULL))
        {
            if (tmp->body.links.next == block)
                return tmp;
            tmp = tmp->body.links.next;
            if (tmp == &sf_free_list_heads[index])
                break;
        }
        index++;
    }
    return NULL;
}

struct sf_block* find_next(sf_block* block)
{
    if (block == NULL)
        return NULL;
    int index = 0;

    while (index < NUM_FREE_LISTS - 1)
    {
        sf_block* tmp = &sf_free_list_heads[index];
        while (tmp && (tmp->body.links.prev) && (tmp->body.links.prev != NULL))
        {
            if (tmp->body.links.prev == block)
                return tmp;
            tmp = tmp->body.links.prev;
            if (tmp == &sf_free_list_heads[index])
                break;
        }
        index++;
    }
    return NULL;
}

struct sf_block* find_and_set_fit(int index, sf_block* insert_block)
{
    sf_block* old_block = &sf_free_list_heads[index];
    size_t insert_block_size = (insert_block->header >> 4) & 0xFFFFFFF;
    while ( old_block && (old_block->body.links.next) && (old_block->body.links.next != &sf_free_list_heads[index]))
    {
        size_t block_size = (old_block->header >> 4) & 0xFFFFFFF;
        size_t next_block_size = (old_block->body.links.next->header >> 4) & 0xFFFFFFF;
        if (block_size < insert_block_size && next_block_size >= insert_block_size)
            break;
        old_block = old_block->body.links.next;
    }
    struct sf_block* old_ptrs = old_block->body.links.next;
    struct sf_block* new_ptrs = insert_block->body.links.next;

    old_ptrs = old_ptrs == NULL ? &sf_free_list_heads[index] : old_ptrs;

    old_block->body.links.next = insert_block;
    old_ptrs->body.links.prev = insert_block;
    insert_block->body.links.next = old_ptrs;
    insert_block->body.links.prev->body.links.next = new_ptrs;
    new_ptrs->body.links.prev = insert_block->body.links.prev;
    insert_block->body.links.prev = old_block;

    old_block->prev_footer = old_block->body.links.prev->header;
    insert_block->prev_footer = insert_block->body.links.prev->header;
    new_ptrs->prev_footer = new_ptrs->body.links.prev->header;
    old_ptrs->prev_footer = old_ptrs->body.links.prev->header;

    old_block->body.links.next->prev_footer = old_block->header;
    insert_block->body.links.next->prev_footer = insert_block->header;
    new_ptrs->body.links.next->prev_footer = new_ptrs->header;
    old_ptrs->body.links.next->prev_footer = old_ptrs->header;

    return old_block;
}

size_t set_payload_size(size_t size, size_t header)
{
    size_t s1 = 0;
    size_t s2 = size;
    while (s2 > 0)
    {
        s1++;
        s2 >>= 1;
    }
    s1 = SF_SIZES.HEADER_WIDTH - s1;
    return (size << s1) | header;
}

size_t set_block_size(size_t size)
{
    return (size << 4);
}

int findBetweenFibonacci(int n, int per_size, int* left, int* right, int limit)
{
    if (n < 0)
        return -1;
    if (n == 0)
    {
        *left = 0;
        *right = 0;
        return 0;
    }
    int temp = *left = 0;
    *right = 1;
    int i = 0;
    while (temp * per_size < n)
    {
        if (i > limit)
        {
            *left = -1;
            *right = -1;
            break;
        }
        temp = *left + *right;
        *left = *right;
        *right = temp;
        i++;
    }
    return i;
}


struct sf_block* get_free_block(int index, size_t payload_size, size_t size)
{
    if (index < 0 || index >= NUM_FREE_LISTS)
        return NULL;
    struct sf_block* result = &sf_free_list_heads[index];
    struct sf_block* tmp = result;
    int tmp_index = index;
    // check the current index for a free block
    while (tmp && (tmp->body.links.next) && (tmp->body.links.next != NULL))
    {
        size_t block_size = (tmp->header >> 4) & 0xFFFFFFF;
        size_t allocated_status = tmp->header & 0x8;

        if (!allocated_status && block_size >= size)
        {
            // might need to change this
            // check if the block can be split
            if (block_size - size >= SF_SIZES.MIN_BLOCK_SIZE)
            {
                // split the block
                struct sf_block* new_block = (void*)tmp + size;
                new_block->header = (block_size - size) << 4;
                tmp->body.links.next->body.links.prev = new_block;
                new_block->body.links.next = tmp->body.links.next;
                new_block->body.links.prev = tmp;
                tmp->body.links.next = new_block;
                tmp->header = set_block_size(size);
                new_block->prev_footer = tmp->header;
                new_block->body.links.next->prev_footer = new_block->header;
            }
            // set the payload size
            tmp->header = set_payload_size(payload_size, tmp->header);
            // move to the best fit place
            find_and_set_fit(tmp_index, tmp);
            // set the allocation bit
            tmp->header |= 0x8;
            tmp->body.links.next->prev_footer = tmp->header;
            // set the header of the next block to have the prev allocated bit
            tmp->body.links.next->header |= 0x4;
            tmp->body.links.next->prev_footer = tmp->body.links.next->header;
            return tmp;
        }
        tmp = tmp->body.links.next;
        if (tmp == result)
            break;
    }
    // check the wilderness list for a free block
    tmp = result = &sf_free_list_heads[NUM_FREE_LISTS - 1];
    while (tmp && (tmp->body.links.next) && (tmp->body.links.next != NULL))
    {
        size_t block_size = (tmp->header >> 4) & 0xFFFFFFF;
        size_t allocated_status = tmp->header & 0x8;

        if (!allocated_status && block_size >= size)
        {
            // check if the wilderness block can be split
            if (block_size - size >= SF_SIZES.MIN_BLOCK_SIZE)
            {
                // split the block
                struct sf_block* new_block = (void*)tmp + size;
                new_block->header = (block_size - size) << 4;
                tmp->body.links.next->body.links.prev = new_block;
                new_block->body.links.next = tmp->body.links.next;
                new_block->body.links.prev = tmp;
                tmp->body.links.next = new_block;
                tmp->header = set_block_size(size);
                new_block->prev_footer = tmp->header;
                new_block->body.links.next->prev_footer = new_block->header;
            }
            // set the payload size
            tmp->header = set_payload_size(payload_size, tmp->header);
            // move to the best fit place
            find_and_set_fit(tmp_index, tmp);
            // set the allocation bit
            tmp->header |= 0x8;
            tmp->body.links.next->prev_footer = tmp->header;
            // set the header of the next block to have the prev allocated bit
            tmp->body.links.next->header |= 0x4;
            return tmp;
        }
        tmp = tmp->body.links.next;
        if (tmp == result)
            break;
    }
    // no free blocks found, grow the heap
    struct sf_block* heap = sf_mem_grow();
    if (heap == NULL)
        return NULL;
    heap->header = (PAGE_SZ << 4);
    heap->body.links.next = NULL;
    heap->body.links.prev = NULL;
    heap->prev_footer = 0;
    // add the new block to the free list
    // by iterating through the wilderness list
    // and adding it to the end
    result = tmp = &sf_free_list_heads[NUM_FREE_LISTS - 1];
    while (tmp->body.links.next != NULL && tmp->body.links.next != result)
        tmp = tmp->body.links.next;
    tmp->body.links.next = heap;
    heap->body.links.prev = tmp;
    heap->body.links.next = result;
    result->body.links.prev = heap;
    heap->prev_footer = tmp->header;
    // now that the heap is added to the wilderness list
    // we can call this function again to find the block
    // that we need
    return get_free_block(index, payload_size, size);
}

int find_allocation_index(int total_bytes, int min_size)
{
    int left, right = 0;
    int result = findBetweenFibonacci(total_bytes, min_size, &left, &right, NUM_FREE_LISTS - 2);
    if (result < 0) // size is too small
        debug("size given is too low");
    if (result > NUM_FREE_LISTS - 2) // size is too large, need to take a wilderness block
        debug("limit reached, need to take a wilderness block");
    debug("To store %d bytes in a block, we need to use a block in the range of (%d, %d] which is in index: %d",
            total_bytes, left, right, result - 1);
    return result - 1;
}

struct sf_block* find_next_free_block(int total_bytes, size_t payload_size, int min_size)
{
    int index = find_allocation_index(total_bytes, min_size);
    if (index < 0)
        return NULL;
    return get_free_block(index, payload_size, total_bytes);
}

int free_allocated_block(sf_block* block)
{
    if (block == NULL)
        return -1;
    // remove the allocation information and the payload size
    size_t block_size = (block->header & 0xFFFFFFF) >> 4;
    size_t allocated_status = (block->header & 0x8) >> 3;
    if (allocated_status == 0)
        return -1;
    block->header = (block_size << 4);
    // find the next and previous blocks
    sf_block* next_block = find_next(block);
    sf_block* prev_block = find_previous(block);

    if (next_block == NULL || prev_block == NULL)
        return -1;

    // repair the links
    next_block->body.links.prev = block;
    prev_block->body.links.next = block;
    block->body.links.next = next_block;
    block->body.links.prev = prev_block;

    // turn off the next block's prev_allocated bit
    next_block->header = (next_block->header >> 3) << 3;

    // set the footer of the next block to the header of the current block
    next_block->prev_footer = block->header;

    // set the footer of the current block to the header of the previous block
    block->prev_footer = prev_block->header;

    return 0;
}