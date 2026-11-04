#ifndef PAGEMANAGER_H
#define PAGEMANAGER_H

#include <stddef.h>

#define PAGE_SIZE 4096

/* Initialize page storage */
int page_manager_initialize(const char *directory);

/* Existing text-based page operations */
int page_manager_write_text(int page_id, const char *text);

int page_manager_read_text(int page_id, char *buffer, int buffer_size);

/* New raw binary page operations */
int page_manager_write_page(
    int page_id,
    const unsigned char *buffer,
    size_t buffer_size
);

int page_manager_read_page(
    int page_id,
    unsigned char *buffer,
    size_t buffer_size
);

/* Display page storage information */
void page_manager_show_info(void);

int page_manager_get_page_count(void);

#endif