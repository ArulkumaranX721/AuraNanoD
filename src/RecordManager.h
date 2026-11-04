#ifndef RECORDMANAGER_H
#define RECORDMANAGER_H

#define RM_MAX_KEY_SIZE 64
#define RM_MAX_VALUE_SIZE 256

#define RM_RECORDS_PER_PAGE 12

typedef struct
{
    char key[RM_MAX_KEY_SIZE];
    char value[RM_MAX_VALUE_SIZE];

} RM_Record;

int record_manager_initialize(void);

int record_manager_insert(const char *key, const char *value);

int record_manager_find(const char *key, RM_Record *result);

int record_manager_delete(const char *key);

void record_manager_show_info(void);

/* Update an existing record */
int record_manager_update(
    const char *key,
    const char *new_value
);

/* Display all records stored in database pages */
int record_manager_list(void);

#endif