#include <stdio.h>
#include "Database.h"
#include "storage.h"
#include "RecordManager.h"
#include "PageManager.h"
#include "BufferPool.h"
#include "TableManager.h"
#include "TableRecordManager.h"
#include "SQL6.h"

void command_loop(DatabaseConfig *database);

int main(void)
{
    DatabaseConfig database = {0};

    printf("========================================\n");
    printf("        LuraXDB DATABASE ENGINE\n");
    printf("========================================\n");

    printf("Project Status : STARTING\n");
    printf("Language       : C17\n");
    printf("Module         : CORE ENGINE\n");

    database_configure(&database);

printf("\nDatabase configuration loaded.\n");


if (!storage_initialize(&database))
{
    printf("\nLuraXDB could not initialize storage.\n");
    return 1;
}

/* Initialize page storage */
if (!page_manager_initialize(database.storage_path))
{
    printf("\nLuraXDB could not initialize page storage.\n");
    return 1;
}

/* Initialize buffer pool */
if (!buffer_pool_initialize())
{
    printf("\nLuraXDB could not initialize the buffer pool.\n");
    return 1;
}

if (!table_manager_initialize())
{
    printf("Could not initialize Table Manager.\n");
    return 1;
}

/* Sprint 6: load user-defined table schemas and row storage. */
if (!sql6_initialize())
{
    printf("Could not initialize Sprint 6 schema engine.\n");
    return 1;
}

if (!table_record_manager_initialize())
{
    printf("Could not initialize Table Record Manager.\n");
    return 1;
}

/* Initialize page-based Record Manager */
if (!record_manager_initialize())
{
    printf("\nLuraXDB could not initialize Record Manager.\n");
    return 1;
}
if (!page_manager_initialize(database.storage_path))
{
    printf("\nLuraXDB could not initialize page storage.\n");
    return 1;
}
if (!buffer_pool_initialize())
{
    printf("\nLuraXDB could not initialize the buffer pool.\n");
    return 1;
}


command_loop(&database);

    

    printf("\nLuraXDB has been shut down.\n");

    return 0;
}