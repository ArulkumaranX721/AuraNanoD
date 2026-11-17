#ifndef TABLERECORDMANAGER_H
#define TABLERECORDMANAGER_H

#include "RecordManager.h"
#include "SQLParser.h"
#include "TableManager.h"

#define TRM_RECORDS_PER_PAGE 12

typedef struct
{
    int table_id;
    char key[RM_MAX_KEY_SIZE];
    char value[RM_MAX_VALUE_SIZE];
} TRM_Record;

int table_record_manager_initialize(void);
int table_record_manager_insert(const char *table_name, const char *key, const char *value);
int table_record_manager_find(const char *table_name, const char *key, TRM_Record *result);
int table_record_manager_update(const char *table_name, const char *key, const char *new_value);
int table_record_manager_delete(const char *table_name, const char *key);
int table_record_manager_update_where(const char *table_name, const char *new_value, int expression_count, const SQLExpressionToken expression[]);
int table_record_manager_delete_where(const char *table_name, int expression_count, const SQLExpressionToken expression[]);
int table_record_manager_list(const char *table_name);
int table_record_manager_select(const char *table_name, int show_key, int show_value, int expression_count, const SQLExpressionToken expression[]);

#endif
