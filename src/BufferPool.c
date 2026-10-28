#include <stdio.h>
#include <string.h>

#include "BufferPool.h"

typedef struct
{
    int page_id;
    int valid;
    unsigned long last_used;
    char data[PAGE_SIZE];
} BufferFrame;

static BufferFrame frames[BUFFER_POOL_SIZE];

static unsigned long access_counter = 0;
static unsigned long cache_hits = 0;
static unsigned long cache_misses = 0;

static int find_frame(int page_id)
{
    for (int i = 0; i < BUFFER_POOL_SIZE; i++)
    {
        if (frames[i].valid && frames[i].page_id == page_id)
        {
            return i;
        }
    }

    return -1;
}

static int select_frame(void)
{
    int oldest_frame = 0;

    for (int i = 0; i < BUFFER_POOL_SIZE; i++)
    {
        if (!frames[i].valid)
        {
            return i;
        }

        if (frames[i].last_used <
            frames[oldest_frame].last_used)
        {
            oldest_frame = i;
        }
    }

    return oldest_frame;
}

static void update_access_time(int frame_index)
{
    frames[frame_index].last_used = ++access_counter;
}

int buffer_pool_initialize(void)
{
    memset(frames, 0, sizeof(frames));

    access_counter = 0;
    cache_hits = 0;
    cache_misses = 0;

    printf("Buffer pool initialized.\n");
    printf("Page frames: %d\n", BUFFER_POOL_SIZE);
    printf("Frame size : %d bytes\n", PAGE_SIZE);

    return 1;
}

int buffer_pool_read_text(int page_id, char *buffer, int buffer_size)
{
    int frame_index;

    if (page_id < 0 || buffer == NULL || buffer_size <= 0)
    {
        printf("Error: Invalid buffer-pool read request.\n");
        return 0;
    }

    frame_index = find_frame(page_id);

    if (frame_index >= 0)
    {
        cache_hits++;

        update_access_time(frame_index);

        size_t length = strlen(frames[frame_index].data);

        if (length >= (size_t)buffer_size)
        {
            printf("Error: Output buffer is too small.\n");
            return 0;
        }

        memcpy(buffer, frames[frame_index].data, length + 1);

        printf("Cache HIT for page %d.\n", page_id);
        return 1;
    }

    cache_misses++;

    frame_index = select_frame();

    char page_data[PAGE_SIZE];

    if (!page_manager_read_text(
            page_id,
            page_data,
            sizeof(page_data)))
    {
        printf("Could not load page %d into the buffer pool.\n",
               page_id);
        return 0;
    }

    frames[frame_index].page_id = page_id;
    frames[frame_index].valid = 1;

    memcpy(
        frames[frame_index].data,
        page_data,
        strlen(page_data) + 1
    );

    update_access_time(frame_index);

    size_t length = strlen(frames[frame_index].data);

    if (length >= (size_t)buffer_size)
    {
        printf("Error: Output buffer is too small.\n");
        return 0;
    }

    memcpy(buffer, frames[frame_index].data, length + 1);

    printf("Cache MISS for page %d. Loaded from disk.\n", page_id);

    return 1;
}

int buffer_pool_write_text(int page_id, const char *text)
{
    int frame_index;

    if (page_id < 0 || text == NULL)
    {
        printf("Error: Invalid buffer-pool write request.\n");
        return 0;
    }

    if (strlen(text) >= PAGE_SIZE)
    {
        printf("Error: Text is too large for one page.\n");
        return 0;
    }

    frame_index = find_frame(page_id);

    /*
     * Write through to disk first.
     * Only update the cache after the disk write succeeds.
     */
    if (!page_manager_write_text(page_id, text))
    {
        printf("Error: Page write failed.\n");
        return 0;
    }

    if (frame_index < 0)
    {
        cache_misses++;
        frame_index = select_frame();

        printf("Cache MISS for page %d.\n", page_id);
    }
    else
    {
        cache_hits++;

        printf("Cache HIT for page %d.\n", page_id);
    }

    frames[frame_index].page_id = page_id;
    frames[frame_index].valid = 1;

    memcpy(
        frames[frame_index].data,
        text,
        strlen(text) + 1
    );

    update_access_time(frame_index);

    printf("Page %d written and cached.\n", page_id);

    return 1;
}

void buffer_pool_show_info(void)
{
    unsigned long total_accesses = cache_hits + cache_misses;

    printf("\nLuraXDB Buffer Pool Statistics\n");
    printf("------------------------------\n");

    printf("Pool Capacity : %d pages\n", BUFFER_POOL_SIZE);
    printf("Page Size     : %d bytes\n", PAGE_SIZE);
    printf("Pool Memory   : %lu bytes\n",
           (unsigned long)sizeof(frames));

    printf("Cache Hits    : %lu\n", cache_hits);
    printf("Cache Misses  : %lu\n", cache_misses);
    printf("Total Accesses: %lu\n", total_accesses);

    if (total_accesses > 0)
    {
        double hit_rate =
            (double)cache_hits * 100.0 /
            (double)total_accesses;

        printf("Hit Rate      : %.2f%%\n", hit_rate);
    }
    else
    {
        printf("Hit Rate      : N/A\n");
    }

    printf("\nCached Pages:\n");

    for (int i = 0; i < BUFFER_POOL_SIZE; i++)
    {
        if (frames[i].valid)
        {
            printf("Frame %d -> Page %d\n",
                   i,
                   frames[i].page_id);
        }
        else
        {
            printf("Frame %d -> Empty\n", i);
        }
    }
}

void buffer_pool_reset_stats(void)
{
    cache_hits = 0;
    cache_misses = 0;

    printf("Buffer pool statistics reset.\n");
}