
#ifndef TABLEMANAGER_H
#define TABLEMANAGER_H

#define TM_MAX_TABLES 32
#define TM_MAX_TABLE_NAME 64

typedef struct
{
    int table_id;
    char table_name[TM_MAX_TABLE_NAME];
    int active;

} TM_Table;

/* Initialize table metadata */
int table_manager_initialize(void);

/* Create a new table */
int table_manager_create(const char *table_name);

/* Find a table */
int table_manager_find(
    const char *table_name,
    TM_Table *result
);

/* Display all tables */
void table_manager_list(void);



#endif

