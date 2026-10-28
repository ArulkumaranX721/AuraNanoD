#include <stdio.h>
#include <string.h>
#include "Database.h"
#include "storage.h"
#include "records.h"

void database_configure(DatabaseConfig *database)
{
    snprintf(database->name,
             sizeof(database->name),
             "%s",
             "LuraXDB");

    snprintf(database->storage_path,
             sizeof(database->storage_path),
             "%s",
             "./data");

    database->initialized = 1;
}

void database_show_config(const DatabaseConfig *database)
{
    printf("\nDatabase Configuration\n");
    printf("----------------------\n");

    printf("Database Name : %s\n", database->name);
    printf("Storage Path  : %s\n", database->storage_path);

    if (database->initialized)
    {
        printf("Configuration : INITIALIZED\n");
    }
    else
    {
        printf("Configuration : NOT INITIALIZED\n");
    }
}