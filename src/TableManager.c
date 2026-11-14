
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "TableManager.h"

#define TM_METADATA_FILE "./data/tables.dat"
#define TM_MAGIC 0x4C585442u
#define TM_VERSION 1

typedef struct
{
    uint32_t magic;
    uint32_t version;
    int table_count;
    TM_Table tables[TM_MAX_TABLES];
} TM_Metadata;

static TM_Metadata metadata;

/* Save table metadata to disk. */
static int save_metadata(void)
{
    FILE *file = fopen(TM_METADATA_FILE, "wb");

    if (file == NULL)
    {
        printf("Table Manager Error: Cannot save metadata.\n");
        return 0;
    }

    size_t written = fwrite(
        &metadata,
        sizeof(TM_Metadata),
        1,
        file);

    fclose(file);

    if (written != 1)
    {
        printf("Table Manager Error: Metadata write failed.\n");
        return 0;
    }

    return 1;
}

/* Initialize the Table Manager. */
int table_manager_initialize(void)
{
    FILE *file = fopen(TM_METADATA_FILE, "rb");

    if (file == NULL)
    {
        memset(&metadata, 0, sizeof(metadata));

        metadata.magic = TM_MAGIC;
        metadata.version = TM_VERSION;
        metadata.table_count = 0;

        if (!save_metadata())
        {
            return 0;
        }

        printf("New table metadata created.\n");
        return 1;
    }

    TM_Metadata loaded;

    size_t read_count = fread(
        &loaded,
        sizeof(TM_Metadata),
        1,
        file);

    fclose(file);

    if (read_count != 1 ||
        loaded.magic != TM_MAGIC ||
        loaded.version != TM_VERSION ||
        loaded.table_count < 0 ||
        loaded.table_count > TM_MAX_TABLES)
    {
        printf("Table Manager Error: Invalid metadata file.\n");
        return 0;
    }

    metadata = loaded;

    printf("Table metadata loaded successfully.\n");
    printf("Existing tables: %d\n", metadata.table_count);

    return 1;
}

/* Create a new table. */
int table_manager_create(const char *table_name)
{
    if (table_name == NULL || table_name[0] == '\0')
    {
        printf("Table Manager Error: Invalid table name.\n");
        return 0;
    }

    if (strlen(table_name) >= TM_MAX_TABLE_NAME)
    {
        printf("Table Manager Error: Table name too long.\n");
        return 0;
    }

    TM_Table existing;

    if (table_manager_find(table_name, &existing))
    {
        printf("Table already exists: %s\n", table_name);
        return 0;
    }

    if (metadata.table_count >= TM_MAX_TABLES)
    {
        printf("Table Manager Error: Maximum table limit reached.\n");
        return 0;
    }

    TM_Table *table =
        &metadata.tables[metadata.table_count];

    table->table_id = metadata.table_count + 1;

    strcpy(table->table_name, table_name);

    table->active = 1;

    metadata.table_count++;

    if (!save_metadata())
    {
        metadata.table_count--;
        memset(table, 0, sizeof(TM_Table));
        return 0;
    }

    printf("Table created successfully.\n");
    printf("Table ID   : %d\n", table->table_id);
    printf("Table Name : %s\n", table->table_name);

    return 1;
}

/* Find a table by name. */
int table_manager_find(
    const char *table_name,
    TM_Table *result)
{
    if (table_name == NULL)
    {
        return 0;
    }

    for (int i = 0; i < metadata.table_count; i++)
    {
        if (metadata.tables[i].active &&
            strcmp(
                metadata.tables[i].table_name,
                table_name) == 0)
        {
            if (result != NULL)
            {
                *result = metadata.tables[i];
            }

            return 1;
        }
    }

    return 0;
}

/* Display all tables. */
void table_manager_list(void)
{
    printf("\nLuraXDB Tables\n");
    printf("========================\n");

    int found = 0;

    for (int i = 0; i < metadata.table_count; i++)
    {
        if (metadata.tables[i].active)
        {
            printf(
                "Table ID : %d | Table Name : %s\n",
                metadata.tables[i].table_id,
                metadata.tables[i].table_name);

            found++;
        }
    }

    printf("------------------------\n");
    printf("Total tables: %d\n", found);
}
