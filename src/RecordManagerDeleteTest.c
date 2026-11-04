#include <stdio.h>

#include "PageManager.h"
#include "BufferPool.h"
#include "RecordManager.h"

int main(void)
{
    RM_Record result;

    printf("\nLuraXDB DELETE AND SLOT REUSE TEST\n");
    printf("==================================\n");

    if (!page_manager_initialize("./data"))
        return 1;

    if (!buffer_pool_initialize())
        return 1;

    if (!record_manager_initialize())
        return 1;

    /* Display existing records */
    printf("\nTEST 1: BEFORE DELETE\n");
    record_manager_list();

    /* Delete existing record */
    printf("\nTEST 2: DELETE RECORD\n");

    if (!record_manager_delete("arul001"))
        return 1;

    /* Verify deletion */
    printf("\nTEST 3: VERIFY DELETION\n");

    if (record_manager_find("arul001", &result))
    {
        printf("ERROR: Deleted record still exists.\n");
        return 1;
    }

    printf("Deleted record is no longer available.\n");

    /* Insert another record */
    printf("\nTEST 4: INSERT NEW RECORD\n");

    if (!record_manager_insert(
            "arul002",
            "LuraXDB Storage Engineer"))
    {
        return 1;
    }

    /* Verify new record */
    printf("\nTEST 5: VERIFY NEW RECORD\n");

    if (!record_manager_find("arul002", &result))
        return 1;

    printf("Key   : %s\n", result.key);
    printf("Value : %s\n", result.value);

    /* Show final records */
    printf("\nTEST 6: FINAL RECORD LIST\n");

    record_manager_list();

    record_manager_show_info();

    printf("\nAll DELETE and SLOT REUSE tests completed.\n");

    return 0;
}