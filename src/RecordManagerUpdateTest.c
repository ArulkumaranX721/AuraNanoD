#include <stdio.h>

#include "PageManager.h"
#include "BufferPool.h"
#include "RecordManager.h"

int main(void)
{
    RM_Record result;

    printf("\nLuraXDB UPDATE AND LIST TEST\n");
    printf("============================\n");

    if (!page_manager_initialize("./data"))
        return 1;

    if (!buffer_pool_initialize())
        return 1;

    if (!record_manager_initialize())
        return 1;

    /* Display existing records */
    printf("\nTEST 1: LIST RECORDS\n");

    record_manager_list();

    /* Update existing record */
    printf("\nTEST 2: UPDATE RECORD\n");

    if (!record_manager_update(
            "arul001",
            "LuraXDB Database Engine Developer"))
    {
        return 1;
    }

    /* Verify updated record */
    printf("\nTEST 3: VERIFY UPDATE\n");

    if (!record_manager_find("arul001", &result))
        return 1;

    printf("Updated Key   : %s\n", result.key);
    printf("Updated Value : %s\n", result.value);

    /* Display all records again */
    printf("\nTEST 4: LIST AFTER UPDATE\n");

    record_manager_list();

    printf("\nAll tests completed.\n");

    return 0;
}