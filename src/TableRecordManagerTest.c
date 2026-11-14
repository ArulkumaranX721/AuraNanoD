
#include <stdio.h>

#include "TableManager.h"
#include "TableRecordManager.h"

int main(void)
{
    printf("LuraXDB TABLE RECORD MANAGER TEST\n");
    printf("==================================\n");

    /* Initialize table metadata first. */
    if (!table_manager_initialize())
    {
        printf("Table Manager initialization failed.\n");
        return 1;
    }

    /* Initialize table-aware record storage. */
    if (!table_record_manager_initialize())
    {
        printf("Table Record Manager initialization failed.\n");
        return 1;
    }

    /* Create test tables if they do not exist. */
    TM_Table table;

    if (!table_manager_find("employees", &table))
    {
        table_manager_create("employees");
    }

    if (!table_manager_find("departments", &table))
    {
        table_manager_create("departments");
    }

    /* Insert employee record if it does not exist. */
    TRM_Record record;

    if (!table_record_manager_find("employees", "101", &record))
    {
        table_record_manager_insert(
            "employees",
            "101",
            "Arul");
    }

    /* Insert department record if it does not exist. */
    if (!table_record_manager_find("departments", "10", &record))
    {
        table_record_manager_insert(
            "departments",
            "10",
            "IT");
    }

    /* Display records separately. */
    table_record_manager_list("employees");

    table_record_manager_list("departments");

    /* Test update. */
    table_record_manager_update(
        "employees",
        "101",
        "LuraXDB Database Engineer");

    /* Verify updated record. */
    if (table_record_manager_find("employees", "101", &record))
    {
        printf("\nUpdated Record\n");
        printf("Table ID : %d\n", record.table_id);
        printf("Key      : %s\n", record.key);
        printf("Value    : %s\n", record.value);
    }

    return 0;
}
