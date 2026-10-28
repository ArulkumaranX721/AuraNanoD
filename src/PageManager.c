#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "PageManager.h"

#define PAGE_FILE_SIZE 512

static char page_file[PAGE_FILE_SIZE];
static int page_count = 0;

int page_manager_initialize(const char *directory)
{
    FILE *file;
    long file_size;
    int written;

    if (directory == NULL)
    {
        printf("Error: Storage directory is missing.\n");
        return 0;
    }

    written = snprintf(
        page_file,
        sizeof(page_file),
        "%s/pages.dat",
        directory
    );

    if (written < 0 || (size_t)written >= sizeof(page_file))
    {
        printf("Error: Page file path is too long.\n");
        return 0;
    }

    file = fopen(page_file, "rb");

    if (file == NULL)
    {
        file = fopen(page_file, "wb");

        if (file == NULL)
        {
            printf("Error: Could not create pages.dat.\n");
            return 0;
        }

        if (fclose(file) != 0)
        {
            printf("Error: Could not close pages.dat.\n");
            return 0;
        }

        page_count = 0;

        printf("Page storage created successfully.\n");
        return 1;
    }

    if (fseek(file, 0, SEEK_END) != 0)
    {
        printf("Error: Could not inspect page file.\n");
        fclose(file);
        return 0;
    }

    file_size = ftell(file);

    if (file_size < 0)
    {
        printf("Error: Could not determine page file size.\n");
        fclose(file);
        return 0;
    }

    if (fclose(file) != 0)
    {
        printf("Error: Could not close page file.\n");
        return 0;
    }

    if (file_size % PAGE_SIZE != 0)
    {
        printf("Error: Page file size is not page-aligned.\n");
        return 0;
    }

    if (file_size / PAGE_SIZE > INT_MAX)
    {
        printf("Error: Page count exceeds the supported limit.\n");
        return 0;
    }

    page_count = (int)(file_size / PAGE_SIZE);

    printf("Page storage initialized successfully.\n");
    printf("Loaded pages: %d\n", page_count);

    return 1;
}

int page_manager_write_text(int page_id, const char *text)
{
    FILE *file;
    unsigned char page[PAGE_SIZE] = {0};
    long offset;
    size_t text_length;

    if (page_id < 0 || text == NULL)
    {
        printf("Error: Invalid page ID or text.\n");
        return 0;
    }

    text_length = strlen(text);

    if (text_length >= PAGE_SIZE)
    {
        printf("Error: Text is too large for one page.\n");
        return 0;
    }

    if (page_id > INT_MAX / PAGE_SIZE)
    {
        printf("Error: Page ID is too large.\n");
        return 0;
    }

    if (page_id > page_count)
    {
        printf("Error: Pages must be created sequentially.\n");
        printf("Next available page ID: %d\n", page_count);
        return 0;
    }

    offset = (long)page_id * PAGE_SIZE;

    memcpy(page, text, text_length);

    file = fopen(page_file, "r+b");

    if (file == NULL)
    {
        file = fopen(page_file, "w+b");
    }

    if (file == NULL)
    {
        printf("Error: Could not open pages.dat.\n");
        return 0;
    }

    if (fseek(file, offset, SEEK_SET) != 0)
    {
        printf("Error: Could not seek to the requested page.\n");
        fclose(file);
        return 0;
    }

    if (fwrite(page, 1, PAGE_SIZE, file) != PAGE_SIZE)
    {
        printf("Error: Could not write the complete page.\n");
        fclose(file);
        return 0;
    }

    if (fclose(file) != 0)
    {
        printf("Error: Could not close the page file.\n");
        return 0;
    }

    if (page_id == page_count)
    {
        page_count++;
    }

    printf("Page %d written successfully.\n", page_id);

    return 1;
}

int page_manager_read_text(int page_id, char *buffer, int buffer_size)
{
    FILE *file;
    unsigned char page[PAGE_SIZE];
    long offset;
    size_t text_length;

    if (page_id < 0 ||
        buffer == NULL ||
        buffer_size <= 0)
    {
        printf("Error: Invalid page ID or output buffer.\n");
        return 0;
    }

    if (page_id >= page_count)
    {
        printf("Error: Page does not exist.\n");
        return 0;
    }

    if (page_id > INT_MAX / PAGE_SIZE)
    {
        printf("Error: Page ID is too large.\n");
        return 0;
    }

    offset = (long)page_id * PAGE_SIZE;

    file = fopen(page_file, "rb");

    if (file == NULL)
    {
        printf("Error: Could not open pages.dat.\n");
        return 0;
    }

    if (fseek(file, offset, SEEK_SET) != 0)
    {
        printf("Error: Could not seek to the requested page.\n");
        fclose(file);
        return 0;
    }

    if (fread(page, 1, PAGE_SIZE, file) != PAGE_SIZE)
    {
        printf("Error: Could not read the complete page.\n");
        fclose(file);
        return 0;
    }

    if (fclose(file) != 0)
    {
        printf("Error: Could not close the page file.\n");
        return 0;
    }

    text_length = 0;

    while (text_length < PAGE_SIZE && page[text_length] != '\0')
    {
        text_length++;
    }

    if (text_length >= (size_t)buffer_size)
    {
        printf("Error: Output buffer is too small.\n");
        return 0;
    }

    memcpy(buffer, page, text_length);
    buffer[text_length] = '\0';

    return 1;
}

void page_manager_show_info(void)
{
    printf("\nPage Storage Information\n");
    printf("------------------------\n");
    printf("Page File    : %s\n", page_file);
    printf("Page Size    : %d bytes\n", PAGE_SIZE);
    printf("Pages Stored : %d\n", page_count);
    printf("Storage Type : Fixed-size pages\n");
    printf("Storage Can be accessed through the allocated memory Size.");
}