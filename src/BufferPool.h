#ifndef BUFFERPOOL_H
#define BUFFERPOOL_H

#include "PageManager.h"

#define BUFFER_POOL_SIZE 4

int buffer_pool_initialize(void);

int buffer_pool_read_text(int page_id, char *buffer, int buffer_size);

int buffer_pool_write_text(int page_id, const char *text);

void buffer_pool_show_info(void);

void buffer_pool_reset_stats(void);

#endif