
#include <stdio.h>
#include "TableManager.h"

int main(void)
{
    printf("LuraXDB TABLE MANAGER TEST\n");
    printf("==========================\n");

    if (!table_manager_initialize())
    {
        printf("Initialization failed.\n");
        return 1;
    }

    table_manager_create("employees");
    table_manager_create("departments");

    table_manager_list();

    TM_Table table;

    if (table_manager_find("employees", &table))
    {
        printf("\nTable found successfully.\n");
        printf("ID   : %d\n", table.table_id);
        printf("Name : %s\n", table.table_name);
    }
    else
    {
        printf("Table not found.\n");
    }

    return 0;
}
