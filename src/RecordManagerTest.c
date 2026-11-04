#include <stdio.h>
#include <string.h>

#include "PageManager.h"
#include "BufferPool.h"
#include "RecordManager.h"

int main(void)
{
    RM_Record result;

    printf("\nLuraXDB Record Manager Test\n");
    printf("===========================\n");

    /* Initialize storage first */
    if (!page_manager_initialize("./data"))
        return 1;

    if (!buffer_pool_initialize())
        return 1;

    if (!record_manager_initialize())
        return 1;

    /* Insert */
    if (!record_manager_insert("arul001", "LuraXDB Developer"))
        return 1;

    /* Retrieve */
    if (!record_manager_find("arul001", &result))
        return 1;

    printf("\nRetrieved Record\n");
    printf("Key   : %s\n", result.key);
    printf("Value : %s\n", result.value);

    /* Display statistics */
    record_manager_show_info();
    buffer_pool_show_info();

    /* Test persistence by reinitializing the in-memory cache */
    printf("\nTesting persistence...\n");

    buffer_pool_initialize();
    record_manager_initialize();

    if (!record_manager_find("arul001", &result))
    {
        printf("Persistence test failed.\n");
        return 1;
    }

    printf("Persistence test passed.\n");
    printf("Key   : %s\n", result.key);
    printf("Value : %s\n", result.value);

    return 0;
}