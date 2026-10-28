#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include "records.h"
#define RECORD_FILE_SIZE 512

static Record records[MAX_RECORDS];
static int record_count = 0;
static char record_file[RECORD_FILE_SIZE];

static int save_records(void)
{
    FILE *file = fopen(record_file, "w");

    if (file == NULL)
    {
        printf("Error: Could not open the record file for writing.\n");
        return 0;
    }

    for (int i = 0; i < record_count; i++)
    {
        if (fprintf(file, "%s\t%s\n",
                    records[i].key,
                    records[i].value) < 0)
        {
            printf("Error: Could not write a record.\n");
            fclose(file);
            return 0;
        }
    }

    if (fclose(file) != 0)
    {
        printf("Error: Could not close the record file properly.\n");
        return 0;
    }

    return 1;
}

int record_store_initialize(const char *directory)
{
    FILE *file;
    char line[KEY_SIZE + VALUE_SIZE + 4];

    int written = snprintf(
        record_file,
        sizeof(record_file),
        "%s/records.dat",
        directory
    );

    if (written < 0 || (size_t)written >= sizeof(record_file))
    {
        printf("Error: Record file path is too long.\n");
        return 0;
    }

    file = fopen(record_file, "r");

    if (file == NULL)
    {
        record_count = 0;

        file = fopen(record_file, "w");

        if (file == NULL)
        {
            printf("Error: Could not create records.dat.\n");
            return 0;
        }

        if (fclose(file) != 0)
        {
            printf("Error: Could not close records.dat.\n");
            return 0;
        }

        printf("Record storage created successfully.\n");
        return 1;
    }

    record_count = 0;

    while (fgets(line, sizeof(line), file) != NULL)
    {
        char *separator;
        size_t length;

        if (record_count >= MAX_RECORDS)
        {
            printf("Error: Record limit exceeded.\n");
            fclose(file);
            return 0;
        }

        length = strlen(line);

        if (length > 0 && line[length - 1] == '\n')
        {
            line[length - 1] = '\0';
        }

        separator = strchr(line, '\t');

        if (separator == NULL)
        {
            printf("Error: Invalid record format in records.dat.\n");
            fclose(file);
            return 0;
        }

        *separator = '\0';

        if (strlen(line) >= KEY_SIZE ||
            strlen(separator + 1) >= VALUE_SIZE ||
            line[0] == '\0')
        {
            printf("Error: Invalid record length or empty key.\n");
            fclose(file);
            return 0;
        }

        strcpy(records[record_count].key, line);
        strcpy(records[record_count].value, separator + 1);

        record_count++;
    }

    if (ferror(file))
    {
        printf("Error: Could not read records.dat completely.\n");
        fclose(file);
        return 0;
    }

    if (fclose(file) != 0)
    {
        printf("Error: Could not close records.dat properly.\n");
        return 0;
    }

    printf("Loaded %d record(s) from disk.\n", record_count);

    return 1;
}

int record_store_put(const char *key, const char *value)
{
    if (key == NULL || value == NULL ||
        key[0] == '\0' ||
        strlen(key) >= KEY_SIZE ||
        strlen(value) >= VALUE_SIZE ||
        strchr(key, '\t') != NULL ||
        strchr(value, '\t') != NULL ||
        strchr(key, '\n') != NULL ||
        strchr(value, '\n') != NULL ||
        strchr(key, '\r') != NULL ||
        strchr(value, '\r') != NULL)
    {
        printf("Error: Invalid key or value.\n");
        return 0;
    }

    for (int i = 0; i < record_count; i++)
    {
        if (strcmp(records[i].key, key) == 0)
        {
            char old_value[VALUE_SIZE];

            strcpy(old_value, records[i].value);
            strcpy(records[i].value, value);

            if (!save_records())
            {
                strcpy(records[i].value, old_value);
                return 0;
            }

            printf("Record updated successfully.\n");
            return 1;
        }
    }

    if (record_count >= MAX_RECORDS)
    {
        printf("Error: Maximum record count reached.\n");
        return 0;
    }

    strcpy(records[record_count].key, key);
    strcpy(records[record_count].value, value);

    record_count++;

    if (!save_records())
    {
        record_count--;
        return 0;
    }

    printf("Record stored successfully.\n");
    return 1;
}

int record_store_get(const char *key, char *value, int value_size)
{
    if (key == NULL || value == NULL || value_size <= 0)
    {
        return 0;
    }

    for (int i = 0; i < record_count; i++)
    {
        if (strcmp(records[i].key, key) == 0)
        {
            if (strlen(records[i].value) >= (size_t)value_size)
            {
                printf("Error: Output buffer is too small.\n");
                return 0;
            }

            strcpy(value, records[i].value);
            return 1;
        }
    }

    return 0;
}

int record_store_delete(const char *key)
{
    if (key == NULL)
    {
        return 0;
    }

    for (int i = 0; i < record_count; i++)
    {
        if (strcmp(records[i].key, key) == 0)
        {
            Record deleted_record = records[i];

            for (int j = i; j < record_count - 1; j++)
            {
                records[j] = records[j + 1];
            }

            record_count--;

            if (!save_records())
            {
                for (int j = record_count; j > i; j--)
                {
                    records[j] = records[j - 1];
                }

                records[i] = deleted_record;
                record_count++;

                return 0;
            }

            printf("Record deleted successfully.\n");
            return 1;
        }
    }

    printf("Error: Key not found.\n");
    return 0;
}

int record_store_list(void)
{
    if (record_count == 0)
    {
        printf("No records found.\n");
        return 1;
    }

    printf("\nStored Records\n");
    printf("--------------------------------\n");

    for (int i = 0; i < record_count; i++)
    {
        printf("Key: %-20s Value: %s\n",
               records[i].key,
               records[i].value);
    }

    return 1;
}