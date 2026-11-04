#ifndef BUFFERPOOL_H
#define BUFFERPOOL_H

#include "PageManager.h"

#define BUFFER_POOL_SIZE 4

int buffer_pool_initialize(void);

/* Existing text operations */
int buffer_pool_read_text(int page_id, char *buffer, int buffer_size);

int buffer_pool_write_text(int page_id, const char *text);

/* New raw binary operations */
int buffer_pool_read_page(
    int page_id,
    unsigned char *buffer,
    size_t buffer_size
);

int buffer_pool_write_page(
    int page_id,
    const unsigned char *buffer,
    size_t buffer_size
);

/* Statistics */
void buffer_pool_show_info(void);

void buffer_pool_reset_stats(void);

#endif