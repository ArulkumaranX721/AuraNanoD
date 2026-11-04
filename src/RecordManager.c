#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "RecordManager.h"
#include "BufferPool.h"
#include "PageManager.h"

/* Identify pages belonging to the Record Manager */
#define RM_PAGE_MAGIC 0x4C585244u
#define RM_PAGE_VERSION 1u

/*
 * Page layout:
 *
 * Header       : 16 bytes
 * Records      : 12 x 320 = 3840 bytes
 * Reserved     : 240 bytes
 * Total        : 4096 bytes
 */

typedef struct
{
    uint32_t magic;
    uint32_t page_id;
    uint32_t record_count;
    uint32_t version;

    RM_Record records[RM_RECORDS_PER_PAGE];

    unsigned char reserved[
        PAGE_SIZE
        - (4 * sizeof(uint32_t))
        - (RM_RECORDS_PER_PAGE * sizeof(RM_Record))
    ];

} RM_Page;

_Static_assert(sizeof(RM_Record) == 320,
               "RM_Record must be exactly 320 bytes");

_Static_assert(sizeof(RM_Page) == PAGE_SIZE,
               "RM_Page must be exactly 4096 bytes");


static int total_records = 0;
static int manager_initialized = 0;




/* Check whether a page belongs to Record Manager */
static int is_record_page(const RM_Page *page, int page_id)
{
    return page->magic == RM_PAGE_MAGIC &&
           page->page_id == (uint32_t)page_id &&
           page->version == RM_PAGE_VERSION &&
           page->record_count <= RM_RECORDS_PER_PAGE;
}


/* Read one Record Manager page */
static int read_record_page(int page_id, RM_Page *page)
{
    unsigned char buffer[PAGE_SIZE];

    if (!buffer_pool_read_page(page_id, buffer, PAGE_SIZE))
    {
        return 0;
    }

    memcpy(page, buffer, PAGE_SIZE);

    return is_record_page(page, page_id);
}


/* Write one Record Manager page */
static int write_record_page(int page_id, const RM_Page *page)
{
    unsigned char buffer[PAGE_SIZE];

    memcpy(buffer, page, PAGE_SIZE);

    return buffer_pool_write_page(page_id, buffer, PAGE_SIZE);
}


/* Initialize a new empty record page */
static void initialize_record_page(RM_Page *page, int page_id)
{
    memset(page, 0, sizeof(RM_Page));

    page->magic = RM_PAGE_MAGIC;
    page->page_id = (uint32_t)page_id;
    page->record_count = 0;
    page->version = RM_PAGE_VERSION;
}


/* Validate record input */
static int validate_record(const char *key, const char *value)
{
    if (key == NULL || value == NULL ||
        key[0] == '\0' ||
        strlen(key) >= RM_MAX_KEY_SIZE ||
        strlen(value) >= RM_MAX_VALUE_SIZE ||
        strchr(key, '\t') != NULL ||
        strchr(value, '\t') != NULL ||
        strchr(key, '\n') != NULL ||
        strchr(value, '\n') != NULL)
    {
        printf("Error: Invalid record key or value.\n");
        return 0;
    }

    return 1;
}


/* Find a key and return its page and slot */
static int locate_record(
    const char *key,
    int *found_page,
    int *found_slot
)
{
    int page_count = page_manager_get_page_count();

    for (int page_id = 0; page_id < page_count; page_id++)
    {
        RM_Page page;

        if (!read_record_page(page_id, &page))
        {
            continue;
        }

        for (int slot = 0; slot < RM_RECORDS_PER_PAGE; slot++)
        {
            if (page.records[slot].key[0] != '\0' &&
                strcmp(page.records[slot].key, key) == 0)
            {
                *found_page = page_id;
                *found_slot = slot;
                return 1;
            }
        }
    }

    return 0;
}


/* Initialize Record Manager */
int record_manager_initialize(void)
{
    int page_count = page_manager_get_page_count();

    total_records = 0;

    for (int page_id = 0; page_id < page_count; page_id++)
    {
        RM_Page page;

        if (!read_record_page(page_id, &page))
        {
            continue;
        }

        total_records += (int)page.record_count;
    }

    manager_initialized = 1;

    printf("\nLuraXDB Record Manager Initialized\n");
    printf("---------------------------------\n");
    printf("Existing pages     : %d\n", page_count);
    printf("Records found      : %d\n", total_records);
    printf("Records per page   : %d\n", RM_RECORDS_PER_PAGE);
    printf("Record size        : %lu bytes\n",
           (unsigned long)sizeof(RM_Record));
    printf("Page size          : %d bytes\n", PAGE_SIZE);

    return 1;
}


/* Insert a new record */
int record_manager_insert(const char *key, const char *value)
{
    int page_count;
    int found_page;
    int found_slot;

    if (!manager_initialized)
    {
        printf("Error: Record Manager is not initialized.\n");
        return 0;
    }

    if (!validate_record(key, value))
    {
        return 0;
    }

    if (locate_record(key, &found_page, &found_slot))
    {
        printf("Error: Record key already exists.\n");
        return 0;
    }

    page_count = page_manager_get_page_count();

    /* Look for an existing Record Manager page with free slots */
    for (int page_id = 0; page_id < page_count; page_id++)
    {
        RM_Page page;

        if (!read_record_page(page_id, &page))
        {
            continue;
        }

        for (int slot = 0; slot < RM_RECORDS_PER_PAGE; slot++)
        {
            if (page.records[slot].key[0] == '\0')
            {
                strcpy(page.records[slot].key, key);
                strcpy(page.records[slot].value, value);

                page.record_count++;

                if (!write_record_page(page_id, &page))
                {
                    printf("Error: Could not save record.\n");
                    return 0;
                }

                total_records++;

                printf("Record inserted successfully.\n");
                printf("Page ID : %d\n", page_id);
                printf("Slot ID : %d\n", slot);

                return 1;
            }
        }
    }

    /* No available slot: create a new page */
    {
        RM_Page page;
        int new_page_id = page_manager_get_page_count();

        initialize_record_page(&page, new_page_id);

        strcpy(page.records[0].key, key);
        strcpy(page.records[0].value, value);

        page.record_count = 1;

        if (!write_record_page(new_page_id, &page))
        {
            printf("Error: Could not create record page.\n");
            return 0;
        }

        total_records++;

        printf("New record page created successfully.\n");
        printf("Page ID : %d\n", new_page_id);
        printf("Slot ID : 0\n");
    }

    return 1;
}


/* Find a record by key */
int record_manager_find(const char *key, RM_Record *result)
{
    int page_id;
    int slot;

    if (!manager_initialized || key == NULL || result == NULL)
    {
        return 0;
    }

    if (!locate_record(key, &page_id, &slot))
    {
        printf("Error: Record not found.\n");
        return 0;
    }

    {
        RM_Page page;

        if (!read_record_page(page_id, &page))
        {
            printf("Error: Could not read record page.\n");
            return 0;
        }

        *result = page.records[slot];
    }

    return 1;
}


/* Delete a record */
int record_manager_delete(const char *key)
{
    int page_id;
    int slot;
    RM_Page page;

    if (!manager_initialized || key == NULL)
    {
        return 0;
    }

    if (!locate_record(key, &page_id, &slot))
    {
        printf("Error: Record not found.\n");
        return 0;
    }

    if (!read_record_page(page_id, &page))
    {
        printf("Error: Could not read record page.\n");
        return 0;
    }

    memset(&page.records[slot], 0, sizeof(RM_Record));

    page.record_count--;

    if (!write_record_page(page_id, &page))
    {
        printf("Error: Could not save deleted record.\n");
        return 0;
    }

    total_records--;

    printf("Record deleted successfully.\n");
    printf("Page ID : %d\n", page_id);
    printf("Slot ID : %d\n", slot);

    return 1;
}

/* Update an existing record */
int record_manager_update(const char *key, const char *new_value)
{
    int page_id;
    int slot;
    RM_Page page;

    if (!manager_initialized)
    {
        printf("Error: Record Manager is not initialized.\n");
        return 0;
    }

    if (key == NULL || key[0] == '\0' ||
        new_value == NULL ||
        strlen(key) >= RM_MAX_KEY_SIZE ||
        strlen(new_value) >= RM_MAX_VALUE_SIZE ||
        strchr(key, '\t') != NULL ||
        strchr(new_value, '\t') != NULL ||
        strchr(key, '\n') != NULL ||
        strchr(new_value, '\n') != NULL)
    {
        printf("Error: Invalid key or new value.\n");
        return 0;
    }

    if (!locate_record(key, &page_id, &slot))
    {
        printf("Error: Record not found.\n");
        return 0;
    }

    if (!read_record_page(page_id, &page))
    {
        printf("Error: Could not read record page.\n");
        return 0;
    }

    /* Update only the value; preserve the key */
    memset(page.records[slot].value, 0, RM_MAX_VALUE_SIZE);

    strcpy(page.records[slot].value, new_value);

    if (!write_record_page(page_id, &page))
    {
        printf("Error: Could not save updated record.\n");
        return 0;
    }

    printf("Record updated successfully.\n");
    printf("Page ID : %d\n", page_id);
    printf("Slot ID : %d\n", slot);

    return 1;
}

/* Display Record Manager information */
void record_manager_show_info(void)
{
    printf("\nLuraXDB Record Manager Information\n");
    printf("----------------------------------\n");

    printf("Total Records       : %d\n", total_records);
    printf("Total Pages         : %d\n",
           page_manager_get_page_count());

    printf("Records Per Page    : %d\n",
           RM_RECORDS_PER_PAGE);

    printf("Record Size         : %lu bytes\n",
           (unsigned long)sizeof(RM_Record));

    printf("Page Size           : %d bytes\n", PAGE_SIZE);
}
/* Display all records stored in database pages */
int record_manager_list(void)
{
    int page_count;
    int displayed_records = 0;

    if (!manager_initialized)
    {
        printf("Error: Record Manager is not initialized.\n");
        return 0;
    }

    page_count = page_manager_get_page_count();

    printf("\nLuraXDB Stored Records\n");
    printf("======================\n");

    for (int page_id = 0; page_id < page_count; page_id++)
    {
        RM_Page page;

        if (!read_record_page(page_id, &page))
        {
            continue;
        }

        for (int slot = 0; slot < RM_RECORDS_PER_PAGE; slot++)
        {
            RM_Record *record = &page.records[slot];

            if (record->key[0] != '\0')
            {
                printf("\nPage ID : %d\n", page_id);
                printf("Slot ID : %d\n", slot);
                printf("Key     : %s\n", record->key);
                printf("Value   : %s\n", record->value);

                displayed_records++;
            }
        }
    }

    printf("\n----------------------\n");
    printf("Total records: %d\n", displayed_records);

    return 1;
}