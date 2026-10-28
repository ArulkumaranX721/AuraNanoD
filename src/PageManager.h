#ifndef PAGEMANAGER_H
#define PAGEMANAGER_H

#define PAGE_SIZE 4096

int page_manager_initialize(const char *directory);

int page_manager_write_text(int page_id, const char *text);

int page_manager_read_text(int page_id, char *buffer, int buffer_size);

void page_manager_show_info(void);

#endif