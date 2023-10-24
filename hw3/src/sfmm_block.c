#include "sfmm_block.h"

static size_t sfmm_peak_utilization = 0;

sfmm_sizes SFMM_SIZES = {
    .HEADER_SIZE = SFMM_HEADER_SIZE,
    .FOOTER_SIZE = SFMM_FOOTER_SIZE,
    .ALIGNMENT_SIZE = SFMM_ALIGNMENT_SIZE,
    .MIN_BLOCK_SIZE = SFMM_MIN_BLOCK_SIZE,
    .HEADER_WIDTH = SFMM_HEADER_WIDTH,
    .PAYLOAD_WIDTH = SFMM_PAYLOAD_WIDTH
};

size_t set_header(size_t payload_size, size_t block_size, size_t alloc, size_t prev_alloc);

#ifdef DEBUG
#define sf_heap(x...) debug(x);sf_show_heap();
#else
#define sf_heap(x...)
#endif

void init_free_lists()
{
    for (int i = 0; i < NUM_FREE_LISTS; i++)
    {
        sf_block* tmp = &sf_free_list_heads[i];
        if (tmp == NULL)
            continue;
        if (tmp->body.links.next == NULL)
            tmp->body.links.next = tmp;
        if (tmp->body.links.prev == NULL)
            tmp->body.links.prev = tmp;
    }
}

int fibber(int n, int a, int b)
{
    if (n == 0)
        return a;
    if (n == 1)
        return b;
    return fibber(n - 1, b, a + b);
}

int fib(int n)
{
    return fibber(n, 1, 2);
}

int find_between_fibonacci(int n, int* left, int* right, int limit)
{
    if (n < 0)
        return -1;
    if (n == 0)
    {
        *left = 0;
        *right = 0;
        return 0;
    }
    *left = 0;
    *right = 1;
    int i = 0;
    int temp = 0;
    size_t per_size = SFMM_SIZES.MIN_BLOCK_SIZE;

    while (((temp = fib(i)) > -1) && (temp * per_size < n))
    {
        if (i > limit)
        {
            *left = -1;
            *right = -1;
            break;
        }
        *left = *right;
        *right = temp;
        i++;
    }
    // return in proper array index mode
    return i;
}

struct sf_block* insert_after(struct sf_block* prev, struct sf_block* next)
{
    if (prev == NULL || next == NULL)
        return NULL;
    sf_block* tmp_next = prev->body.links.next == NULL ? prev : prev->body.links.next;

    // do the linking
    prev->body.links.next = next;
    next->body.links.prev = prev;
    next->body.links.next = tmp_next;
    tmp_next->body.links.prev = next;

    // update the prev_footer fields
    next->prev_footer = prev->header;
    tmp_next->prev_footer = next->header;
    return next;
}

sf_block* touch_for_heap_update(sf_block* block)
{
    if (block == NULL)
        return NULL;
    size_t og_header = block->header;
    size_t block_size = block->header & 0xFFFFFF3;
    sf_block* next = (sf_block*)((void*)block + block_size);
    sf_block* prev = (sf_block*)((void*)block - (block->prev_footer & 0xFFFFFF3));
    void* end = sf_mem_end();
    void*start = sf_mem_start();
    if ((void*)next < start || (void*)next > end)
    {
        // next is not in the heap
        return NULL;
    }
    if ((void*)prev < start || (void*)prev > end)
    {
        // prev is not in the heap
        return NULL;
    }
    size_t is_prev_alloc = (prev->header & 0x8) >> 3;
    if (is_prev_alloc)
    {
        // update the current block's header to reflect prev being allocated
        block->header = block->header | 0x4;
        // update the prev_footer field to reflect the prev block being allocated
        block->prev_footer = prev->header;
        // update the next block's prev_footer to reflect the change
        next->prev_footer = block->header;
    }
    size_t is_current_alloc = (block->header & 0x8) >> 3;
    if (og_header != block->header)
    {
        // the header has changed
        // update the next block's prev_footer to reflect the change
        next->prev_footer = block->header;
    }
    if (is_current_alloc)
    {
        // update the next block's prev_footer to reflect the change
        next->prev_footer = block->header;
        // update the next block's header to reflect the prev block is allocated
        next->header = next->header | 0x4;
    }
    next->prev_footer = block->header;
    size_t is_next_alloc = (next->header & 0x8) >> 3;
    next->header = set_header(peek_payload_size(next), peek_block_size(next), is_next_alloc, is_current_alloc);
    // return the block
    return block;
}

struct sf_block* isolate_block(sf_block* block)
{
    if (block == NULL)
        return NULL;
    sf_block* next = block->body.links.next;
    if (next == NULL)
    {
        // block is not in the list
        return NULL;
    }
    sf_block* prev = block->body.links.prev;
    if (prev == NULL)
    {
        // block is not in the list
        return NULL;
    }

    // destroy the links
    prev->body.links.next = next;
    next->body.links.prev = prev;
    block->body.links.next = NULL;
    block->body.links.prev = NULL;

    // update the prev_footer fields
    touch_for_heap_update(next);

    // return the block
    return block;
}

int find_allocation_index(size_t total_bytes)
{
    int left = 0, right = 0;
    int index = find_between_fibonacci(total_bytes, &left, &right, NUM_FREE_LISTS - 2);
    if (index < 0)
        return -1;
    return index;
}

size_t get_payload_size(size_t payload_size)
{
    int bit_length = 0;
    size_t tmp = payload_size;
    while (tmp > 0)
    {
        tmp = tmp >> 1;
        bit_length++;
    }
    // return left bit shift so that the payload size is HEADER_WIDTH bits - (total bits for payload - current bit length)
    size_t shift = SFMM_SIZES.HEADER_WIDTH - SFMM_SIZES.PAYLOAD_WIDTH;
    return payload_size << shift;
}

size_t get_block_size(size_t block_size)
{
    int bit_length = 0;
    size_t tmp = block_size;
    while (tmp > 0)
    {
        tmp = tmp >> 1;
        bit_length++;
    }
    if (bit_length > (SFMM_SIZES.HEADER_WIDTH - SFMM_SIZES.PAYLOAD_WIDTH))
    {
        debug("Block size is too large");
    }
    // return the block size with space for allocation bits
    return block_size;
}

/**
 * Gets the proper header for the block
 * @param payload_size the size of the payload
 * @param block_size the size of the block
 * @param alloc whether this block is allocated or not
 * @param prev_alloc whether the previous block is allocated or not
*/
size_t set_header(size_t payload_size, size_t block_size, size_t alloc, size_t prev_alloc)
{
    size_t header = get_payload_size(payload_size) | get_block_size(block_size);
    if (alloc)
        header = header | 0x8;
    if (prev_alloc)
        header = header | 0x4;
    return header;
}


sf_block* insert_block(sf_block* block, int index)
{
    if (block == NULL)
        return NULL;
    if (index < 0 || index >= NUM_FREE_LISTS)
        return NULL;
    sf_block* head = &sf_free_list_heads[index];
    if (head == NULL)
        return NULL;
    sf_block* tmp = head->body.links.next;
    if (tmp == NULL || tmp == head)
    {
        // the list is empty
        head->body.links.next = block;
        head->body.links.prev = block;
        block->body.links.next = head;
        block->body.links.prev = head;
        goto empty_list;
    }
    block->body.links.next = tmp;
    block->body.links.prev = head;
    head->body.links.next = block;
    tmp->body.links.prev = block;

empty_list:
    // set the prev_footer field
    touch_for_heap_update(block);
    return block;
}

void* get_more_memory()
{
    // if the start of the heap is equal to the end of the heap, a prologue block is needed
    size_t diff = sf_mem_end() - sf_mem_start();
    void* old_epilogue = diff != 0 ? sf_mem_end() - SFMM_SIZES.ALIGNMENT_SIZE : NULL;
    // grow the memory
    void* new_mem = sf_mem_grow();
    if (new_mem == NULL)
    {
        // no more memory can be allocated
        debug("No more memory can be allocated");
        return NULL;
    }
    // if there is an old epilogue, set the beginning of the new memory to be the old epilogue
    if (old_epilogue != NULL)
        new_mem = old_epilogue;
    // if there is a difference, do not create a prologue block
    if (diff == 0)
    {
        // set the prologue block to be 8 bytes from the start of the heap
        sf_block *prologue = (sf_block*)(sf_mem_start());
        // set the header to be 0, MIN_BLOCK_SIZE, 1, 0
        prologue->header = set_header(0, SFMM_SIZES.MIN_BLOCK_SIZE, 1, 0);
        // set the footer to be the same as the header
        prologue->prev_footer = prologue->header;
    }
    // create an epilogue header
    sf_block *epilogue = (sf_block *)(sf_mem_end() - SFMM_SIZES.ALIGNMENT_SIZE);
    // set the epilogue header to have 0 payload, 0 block size, and 1 for allocation
    epilogue->header = set_header(0, 0, 1, 0);

    // return the start of the new memory
    return diff == 0 ? new_mem + (SFMM_SIZES.MIN_BLOCK_SIZE) : new_mem;
}

int can_coalesce(sf_block* block)
{
    if (block == NULL)
        return 0;
    size_t alloc = (block->header & 0x8) >> 2;
    if (alloc)
        return 0;
    size_t block_size = (block->header & 0xFFFFFF3);
    size_t prev_size = (block->prev_footer & 0xFFFFFF3);
    sf_block* next = (sf_block*)((void*)block + block_size);
    sf_block* prev = (sf_block*)((void*)block - prev_size);
    if ((void*)next < sf_mem_end())
    {
        size_t next_alloc = (next->header & 0x8) >> 3;
        if (!next_alloc)
            return 1;
    }
    if ((void*)prev > sf_mem_start())
    {
        size_t prev_alloc = (prev->header & 0x8) >> 3;
        if (!prev_alloc)
            return 1;
    }
    return 0;
}

sf_block* coalesce(sf_block* main, sf_block* join)
{
    if (main == NULL || join == NULL)
        return NULL;
    sf_block* next_in_heap = (sf_block*)((void*)main + (main->header & 0xFFFFFF3));
    sf_block* prev_in_heap = (sf_block*)((void*)main - (main->prev_footer & 0xFFFFFF3));
    if (next_in_heap != join && prev_in_heap != join)
        return NULL;

    size_t main_alloc = main->header & 0x8;
    size_t join_alloc = join->header & 0x8;
    if (main_alloc || join_alloc)
    {
        // the blocks are allocated, do not touch
        return NULL;
    }
    // get the size of the main block
    size_t main_size = main->header & 0xFFFFFF3;
    // get the size of the join block
    size_t join_size = join->header & 0xFFFFFF3;
    sf_block* higher_in_heap = main > join ? join : main;
    sf_block* lower_in_heap = main < join ? join : main;
    int prev_alloc = higher_in_heap->header & 0x4;
    higher_in_heap->header = set_header(0, main_size + join_size, 0, prev_alloc);
    int index = find_allocation_index(main_size + join_size);
    if (index < 0)
    {
        // the block is too large
        return NULL;
    }
    // insert the block into the proper free list
    sf_block* prev = higher_in_heap->body.links.prev;
    sf_block* next = higher_in_heap->body.links.next;
    prev->body.links.next = next;
    next->body.links.prev = prev;
    next = (&sf_free_list_heads[index])->body.links.next;
    prev = &sf_free_list_heads[index];
    prev->body.links.next = higher_in_heap;
    higher_in_heap->body.links.prev = prev;
    higher_in_heap->body.links.next = next;
    next->body.links.prev = higher_in_heap;

    lower_in_heap = isolate_block(lower_in_heap);

    higher_in_heap->prev_footer = lower_in_heap->prev_footer;
    touch_for_heap_update(higher_in_heap);

    next = (sf_block*)((void*)higher_in_heap + (main_size + join_size));
    next->prev_footer = higher_in_heap->header;

    return higher_in_heap;
}

int do_coalesce()
{
    sf_block* tmp = (sf_block*)(sf_mem_start());
    if (tmp == NULL)
        return 0;
    size_t current_size = tmp->header & 0xFFFFFF3;
    void* end = sf_mem_end() - SFMM_SIZES.ALIGNMENT_SIZE;
    while (((void*)tmp) < end)
    {
        sf_block* next = (sf_block*)((void*)tmp + current_size);
        if (next == NULL)
            return 0;
        if (((void*)next) >= end)
            return 0;
        if (can_coalesce(tmp))
        {
            sf_block* tmp2 = coalesce(tmp, next);
            if (tmp2 == NULL)
            {
                tmp = next;
                current_size = tmp->header & 0xFFFFFF3;
                continue;
            }
            tmp = tmp2;
            sf_heap();
            current_size = tmp->header & 0xFFFFFF3;
            continue;
        }
        tmp = next;
        current_size = tmp->header & 0xFFFFFF3;
        if (current_size < SFMM_SIZES.MIN_BLOCK_SIZE)
            return 0;
    }
    return 1;
}

sf_block* find_last_in_heap()
{
    sf_block* tmp = (sf_block*)(sf_mem_start());
    sf_block* last = tmp;
    if (tmp == NULL)
        return NULL;
    size_t current_size = tmp->header & 0xFFFFFF3;
    if (current_size < SFMM_SIZES.MIN_BLOCK_SIZE)
        return NULL;
    void* end = sf_mem_end() - SFMM_SIZES.ALIGNMENT_SIZE;
    while (((void*)tmp) < end)
    {
        last = tmp;
        tmp = (sf_block*)((void*)tmp + current_size);
        current_size = tmp->header & 0xFFFFFF3;
        if (current_size < SFMM_SIZES.MIN_BLOCK_SIZE)
            return last;
    }
    return last;
}

struct sf_block* get_free_block(int index, size_t payload_size, size_t total_bytes)
{
    if (index < 0 || index >= NUM_FREE_LISTS)
        return NULL;
    struct sf_block* head = &sf_free_list_heads[index];
    if (head == NULL)
        return NULL;
    struct sf_block* tmp = head->body.links.next;
    int temp_index = index;

    while (temp_index < NUM_FREE_LISTS)
    {
        // iterate through the current index to see if a block is available
        while (tmp && (tmp != head))
        {
            // remove the allocation information and look at the remaining 28 bits to see if it is large enough
            size_t block_size = tmp->header & 0xFFFFFF3;
            // look to see if the current block is already allocated
            size_t alloc = tmp->header & 0x8;

            // if not allocated and the block is large enough
            if (!alloc && block_size >= total_bytes)
            {
                // check if the block is large enough to split
                if (block_size - total_bytes >= SFMM_SIZES.MIN_BLOCK_SIZE)
                {
                    // split the block
                    struct sf_block *split_section_block = (void *)tmp + total_bytes;
                    split_section_block->header = set_header(0, block_size - total_bytes, 0, 1);
                    split_section_block->prev_footer = tmp->header;
                    // insert the split section block into the proper free list
                    int index_of_split = find_allocation_index(block_size - total_bytes);
                    insert_block(split_section_block, index_of_split);
                }
                // update the current block's header to reflect the allocation
                // header has the payload size in the first 32 bits,
                // the entire block size in the next 28 bits, and the allocation
                // information in the last 4 bits for a total of 64 bits
                size_t prev_alloc = tmp->body.links.prev->header & 0x8;
                tmp->header = set_header(payload_size, total_bytes, 1, prev_alloc);
                // get the next block
                struct sf_block* next = tmp->body.links.next;
                // update the block
                touch_for_heap_update(tmp);
                // isolate the block
                tmp = isolate_block(tmp);
                // should not happen, means something went wrong
                if (tmp == NULL)
                    return NULL;
                // update the next block
                touch_for_heap_update(next);
                // update the epilogue's prev_footer to reflect the change
                sf_block* last = find_last_in_heap();
                sf_block* epilogue = (sf_block*)(sf_mem_end() - SFMM_SIZES.ALIGNMENT_SIZE);

                epilogue->prev_footer = last->header;
                int should_have_prev = last == (sf_block*)(sf_mem_start()) ? 0 : (last->header & 0x8) >> 3;
                epilogue->header |= (should_have_prev << 2);
                // return the block
                return tmp;
            }
            // if the block is not large enough, move to the next block
            tmp = tmp->body.links.next;
        }
        // if no block is found, move to the next index
        head = &sf_free_list_heads[++temp_index];
        tmp = head->body.links.next;
    }
    // no block was found, call for more memory
    size_t diff = sf_mem_end() - sf_mem_start();
    void* new_mem = get_more_memory();

    if (new_mem == NULL)
    {
        // no more memory can be allocated
        debug("No more memory can be allocated");
        return NULL;
    }
    debug("new mem: %p", new_mem);
    // create a new block from the new memory
    sf_block* new_block = (sf_block*)new_mem;

    // set the header to have 0 payload, PAGE_SZ - (40 if prologue exists) - 16 for the block size, and 0 for allocation
    new_block->header = set_header(0, PAGE_SZ - (diff == 0 ? (SFMM_SIZES.MIN_BLOCK_SIZE + SFMM_SIZES.HEADER_SIZE + SFMM_SIZES.FOOTER_SIZE) : 0), 0, 0);
    // set the footer to be the same as the header
    new_block->prev_footer = diff == 0 ? (SFMM_SIZES.MIN_BLOCK_SIZE) : new_block->prev_footer;
    // add the new block to the last free list
    insert_block(new_block, NUM_FREE_LISTS - 1);
    sf_heap();
    // update the next
    touch_for_heap_update(new_block);
    // update the epilogue
    sf_block* last = find_last_in_heap();
    sf_block* epilogue = (sf_block*)(sf_mem_end() - SFMM_SIZES.ALIGNMENT_SIZE);
    epilogue->prev_footer = last->header;
    int should_have_prev = last == (sf_block*)(sf_mem_start()) ? 0 : (last->header & 0x8) >> 3;
    epilogue->header |= (should_have_prev << 2);

    // perform coalesce
    do_coalesce();
    // call the function again to get the block
    return get_free_block(index, payload_size, total_bytes);
}

double get_utilization()
{
    void* start = sf_mem_start();
    void* end = sf_mem_end();
    if (start == end)
        return 0;
    sf_block* tmp = (sf_block*)start;
    double total = 0.0;
    while ((void*)tmp < (void*)end)
    {
        double current_payload = (double)peek_payload_size(tmp);
        size_t block_size = (double)peek_block_size(tmp);
        if (!block_size)
            break;
        total += current_payload;
        tmp = (sf_block*)((void*)tmp + block_size);
    }
    if (total > sfmm_peak_utilization)
        sfmm_peak_utilization = total;
    return total;
}

// =============================================================
// Below are functions that are visible through the header file
// =============================================================


struct sf_block* find_next_free_block(size_t total_bytes, size_t payload_size)
{
    init_free_lists();
    int index = find_allocation_index(total_bytes);
    if (index < 0)
    {
        // improper number given
        return NULL;
    }
    // get the free block
    struct sf_block* block = get_free_block(index, payload_size, total_bytes);
    if (block == NULL)
    {
        // no block was found
        info("YO? %i", index);
        return NULL;
    }
    get_utilization();
    sf_heap("Allocating %lu bytes", total_bytes);
    // return the block
    return block;
}

int free_allocated_block(sf_block* block)
{
    if (((size_t)block) % SFMM_SIZES.ALIGNMENT_SIZE != 0)
        return -1;
    if (block == NULL)
        return -1;
    // get the size of the block
    size_t block_size = block->header & 0xFFFFFF3;
    // get the allocation information
    size_t alloc = (block->header & 0x8) >> 3;
    // the block is not allocated
    if (!alloc)
        return -1;
    // get the index of the block
    int index = find_allocation_index(block_size);
    // the block is too large
    if (index < 0)
        return -1;

    // get the prev_alloc information
    size_t prev_alloc = (block->header & 0x4) >> 2;
    size_t prev_block_alloc = (block->prev_footer & 0x8) >> 3;
    if ((prev_alloc == 0 && prev_block_alloc != 0) ||
        (prev_alloc != 0 && prev_block_alloc == 0))
    {
        // the prev_alloc information is incorrect
        return -1;
    }
    // set the block to be unallocated
    // insert the block into the free list
    insert_block(block, index);
    block->header = set_header(0, block_size, 0, prev_alloc);
    touch_for_heap_update(block);
    // perform coalesce
    do_coalesce();
    // fix the epilogue
    sf_block* last = find_last_in_heap();
    sf_block* epilogue = (sf_block*)(sf_mem_end() - SFMM_SIZES.ALIGNMENT_SIZE);
    epilogue->prev_footer = last->header;
    int is_prev_alloc = (last->header & 0x8) >> 3;
    epilogue->header = is_prev_alloc == 0 ? epilogue->header & 0xFFFFFFFB : epilogue->header | 0x4;
    // return 0 for success
    return 0;
}

size_t peek_block_size(sf_block* sfb)
{
    if (sfb == NULL)
        return 0;
    return sfb->header & 0xFFFFFF3;
}

size_t peek_payload_size(sf_block* sfb)
{
    if (sfb == NULL)
        return 0;
    return sfb->header >> (SFMM_SIZES.HEADER_WIDTH - SFMM_SIZES.PAYLOAD_WIDTH);
}

int is_allocated(sf_block* sfb)
{
    if (sfb == NULL)
        return -1;
    return (sfb->header & 0x8) >> 3;
}

int can_realloc_without_splinter(sf_block* block, size_t rsize)
{
    if (block == NULL || rsize == 0)
        return 0;
    // size_t block_size = peek_block_size(block);
    size_t payload_size = peek_payload_size(block);
    size_t alloc = (block->header & 0x8) >> 3;
    if (!alloc)
        return -1;
    if (rsize > payload_size)
        return 1;
    if (payload_size == rsize)
        return 2;
    // will cause a splinter
    if ((payload_size - rsize) < SFMM_SIZES.MIN_BLOCK_SIZE)
        return 3;
    // no splinters, reallocated normally
    return 4;
}

sf_block* update_payload_size(sf_block* sfb, size_t update)
{
    if (sfb == NULL)
        return NULL;
    size_t payload_size = update;
    size_t block_size = peek_block_size(sfb);
    size_t alloc = (sfb->header & 0x8) >> 3;
    size_t prev_alloc = (sfb->header & 0x4) >> 2;
    // get the next and prev in heap
    sf_block* next = (sf_block*)((void*)sfb + block_size);
    if ((void*)next > sf_mem_end() - SFMM_SIZES.ALIGNMENT_SIZE)
    {
        // next is not in the heap
        return NULL;
    }
    sfb->header = set_header(payload_size, block_size, alloc, prev_alloc);
    // update the next's prev_footer field
    next->prev_footer = sfb->header;
    return sfb;
}

sf_block* realloc_block(sf_block* sfb, size_t new_block_size, size_t new_payload)
{
    if (sfb == NULL)
        return NULL;
    if (((size_t)sfb) % SFMM_SIZES.ALIGNMENT_SIZE != 0)
        return NULL;
    size_t block_size = peek_block_size(sfb);
    size_t payload_size = new_payload;
    size_t alloc = (sfb->header & 0x8) >> 3;
    size_t prev_alloc = (sfb->header & 0x4) >> 2;
    sf_block* split_section_block = (void *)sfb + new_block_size;
    // get the next and prev in heap
    sfb->header = set_header(payload_size, new_block_size, alloc, prev_alloc);
    split_section_block->prev_footer = sfb->header;
    sf_block* next = (sf_block*)((void*)split_section_block + (block_size - new_block_size));
    if ((void*)next > sf_mem_end() - SFMM_SIZES.ALIGNMENT_SIZE)
    {
        // next is not in the heap
        return NULL;
    }
    // set the header
    split_section_block->header = set_header(0, block_size - new_block_size, 0, 1);
    // update the next's prev_footer field
    next->prev_footer = split_section_block->header;
    // insert the block into the proper free list
    int index = find_allocation_index(block_size - new_block_size);
    if (index < 0)
    {
        // the block is too large
        return NULL;
    }
    // insert the block into the proper free list
    insert_block(split_section_block, index);

    do_coalesce();
    // fix the epilogue
    sf_block* last = find_last_in_heap();
    sf_block* epilogue = (sf_block*)(sf_mem_end() - SFMM_SIZES.ALIGNMENT_SIZE);
    epilogue->prev_footer = last->header;
    int is_prev_alloc = (last->header & 0x8) >> 3;
    epilogue->header = is_prev_alloc == 0 ? epilogue->header & 0xFFFFFFFB : epilogue->header | 0x4;

    get_utilization();
    return sfb;
}

double get_peak_utilization()
{
    return (sfmm_peak_utilization / (double)(sf_mem_end() - sf_mem_start()));
}
// END src/sfmm_block.c