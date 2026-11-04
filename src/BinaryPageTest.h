#include <stdio.h>
#include <string.h>

#include "PageManager.h"
#include "BufferPool.h"

int main(void)
{
    unsigned char write_buffer[PAGE_SIZE];
    unsigned char read_buffer[PAGE_SIZE];

    printf("\nLuraXDB Binary Page Test\n");
    printf("------------------------\n");

    /* Initialize page storage */
    if (!page_manager_initialize("./data"))
    {
        return 1;
    }

    if (!buffer_pool_initialize())
    {
        return 1;
    }

    /* Fill page with binary test data */
    for (int i = 0; i < PAGE_SIZE; i++)
    {
        write_buffer[i] = (unsigned char)(i % 256);
    }

    /* Write binary page */
    if (!buffer_pool_write_page(
            2,
            write_buffer,
            PAGE_SIZE))
    {
        printf("Binary write failed.\n");
        return 1;
    }

    /* Read binary page */
    memset(read_buffer, 0, PAGE_SIZE);

    if (!buffer_pool_read_page(
            2,
            read_buffer,
            PAGE_SIZE))
    {
        printf("Binary read failed.\n");
        return 1;
    }

    /* Compare all 4096 bytes */
    if (memcmp(write_buffer, read_buffer, PAGE_SIZE) == 0)
    {
        printf("\nSUCCESS: All 4096 binary bytes match.\n");
    }
    else
    {
        printf("\nFAILED: Binary page data mismatch.\n");
        return 1;
    }

    buffer_pool_show_info();

    return 0;
}