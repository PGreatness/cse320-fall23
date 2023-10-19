#include "sfmm_analytics.h"

double sfmm_get_fragmentation()
{
    void* start = sf_mem_start();
    void* end = sf_mem_end();

    if (start == end)
        return 0.0;

    sf_block* curr = start;
    double total_payload = 0.0, total_allocated = 0.0;
    while ((void*)curr < (void*)end)
    {
        double current_payload = (double)peek_payload_size(curr);
        double current_block_size = (double)peek_block_size(curr);
        if (current_payload && is_allocated(curr))
        {
            total_payload += current_payload;
            total_allocated += current_block_size;
        }
        if (!current_block_size)
            break;
        curr = (sf_block*)((void*)curr + (int)current_block_size);
    }

    return total_allocated > 0 ? total_payload / total_allocated : 0.0;
}

double sfmm_get_utilization()
{
    void* start = sf_mem_start();
    void* end = sf_mem_end();

    if (start == end)
        return 0.0;

    sf_block* curr = start;
    double total_payload = 0.0;
    while ((void*)curr < (void*)end)
    {
        double current_payload = (double)peek_payload_size(curr);
        double current_block_size = (double)peek_block_size(curr);
        if (current_block_size)
            total_payload += current_payload;
        else break;
        curr = (sf_block*)((void*)curr + (int)current_block_size);
    }

    return total_payload / ((double)((char*)end - (char*)start));
}