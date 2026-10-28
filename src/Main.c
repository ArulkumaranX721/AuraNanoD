#include <stdio.h>
#include "Database.h"
#include "storage.h"
#include "records.h"
#include "PageManager.h"

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

if (!record_store_initialize(database.storage_path))
{
    printf("\nLuraXDB could not initialize record storage.\n");
    return 1;
}

if (!page_manager_initialize(database.storage_path))
{
    printf("\nLuraXDB could not initialize page storage.\n");
    return 1;
}


command_loop(&database);

    

    printf("\nLuraXDB has been shut down.\n");

    return 0;
}