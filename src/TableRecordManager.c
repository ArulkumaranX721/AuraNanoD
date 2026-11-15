
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include "SQLParser.h"
#include "TableRecordManager.h"

#define TRM_FILE "./data/table_records.dat"
#define TRM_MAGIC 0x4C585452u
#define TRM_VERSION 1
#define TRM_MAX_RECORDS 1000


typedef struct
{
    uint32_t magic;
    uint32_t version;
    int record_count;

    TRM_Record records[TRM_MAX_RECORDS];

} TRM_Database;





static TRM_Database database;

/* Find the table ID using its name. */
static int get_table_id(const char *table_name, int *table_id)
{
    if (table_name == NULL || table_id == NULL)
    {
        return 0;
    }

    TM_Table table;

    if (!table_manager_find(table_name, &table))
    {
        printf("Table does not exist: %s\n", table_name);
        return 0;
    }

    *table_id = table.table_id;

    return 1;
}

/* Save all table records to disk. */
static int save_records(void)
{
    FILE *file = fopen(TRM_FILE, "wb");

    if (file == NULL)
    {
        printf("TRM Error: Cannot open record storage file.\n");
        return 0;
    }

    size_t written = fwrite(
        &database,
        sizeof(TRM_Database),
        1,
        file);

    fclose(file);

    if (written != 1)
    {
        printf("TRM Error: Failed to save records.\n");
        return 0;
    }

    return 1;
}

/* Initialize table-aware record storage. */
int table_record_manager_initialize(void)
{
    FILE *file = fopen(TRM_FILE, "rb");

    if (file == NULL)
    {
        memset(&database, 0, sizeof(database));

        database.magic = TRM_MAGIC;
        database.version = TRM_VERSION;
        database.record_count = 0;

        if (!save_records())
        {
            return 0;
        }

        printf("New table record storage created.\n");
        return 1;
    }

    TRM_Database loaded;

    size_t read_count = fread(
        &loaded,
        sizeof(TRM_Database),
        1,
        file);

    fclose(file);

    if (read_count != 1 ||
        loaded.magic != TRM_MAGIC ||
        loaded.version != TRM_VERSION ||
        loaded.record_count < 0 ||
        loaded.record_count > TRM_MAX_RECORDS)
    {
        printf("TRM Error: Invalid record storage file.\n");
        return 0;
    }

    database = loaded;

    printf("Table record storage loaded successfully.\n");
    printf("Existing records: %d\n", database.record_count);

    return 1;
}

/* Insert a record into a specific table. */
int table_record_manager_insert(
    const char *table_name,
    const char *key,
    const char *value)
{
    if (key == NULL || value == NULL ||
        key[0] == '\0' || table_name == NULL)
    {
        printf("TRM Error: Invalid input.\n");
        return 0;
    }

    if (strlen(key) >= RM_MAX_KEY_SIZE ||
        strlen(value) >= RM_MAX_VALUE_SIZE)
    {
        printf("TRM Error: Key or value is too long.\n");
        return 0;
    }

    int table_id;

    if (!get_table_id(table_name, &table_id))
    {
        return 0;
    }

    TRM_Record existing;

    if (table_record_manager_find(table_name, key, &existing))
    {
        printf("TRM Error: Duplicate key in table %s.\n", table_name);
        return 0;
    }

    if (database.record_count >= TRM_MAX_RECORDS)
    {
        printf("TRM Error: Maximum record capacity reached.\n");
        return 0;
    }

    int position = database.record_count;

    TRM_Record *record = &database.records[position];

    memset(record, 0, sizeof(TRM_Record));

    record->table_id = table_id;

    strcpy(record->key, key);
    strcpy(record->value, value);

    database.record_count++;

    if (!save_records())
    {
        database.record_count--;
        memset(record, 0, sizeof(TRM_Record));
        return 0;
    }

    printf("Record inserted successfully.\n");
    printf("Table      : %s\n", table_name);
    printf("Table ID   : %d\n", table_id);
    printf("Key        : %s\n", key);

    return 1;
}

static int compare_values(const char *left, const char *right)
{
    char *end1;
    char *end2;

    long n1 = strtol(left, &end1, 10);
    long n2 = strtol(right, &end2, 10);

    if (left[0] != '\0' &&
        right[0] != '\0' &&
        *end1 == '\0' &&
        *end2 == '\0')
    {
        if (n1 > n2) return 1;
        if (n1 < n2) return -1;
        return 0;
    }

    return strcmp(left, right);
}

/* SQL LIKE matching: % = any sequence, _ = one character */
static int match_like(const char *text, const char *pattern)
{
    if (*pattern == '\0')
        return *text == '\0';

    if (*pattern == '%')
    {
        while (*pattern == '%')
            pattern++;

        if (*pattern == '\0')
            return 1;

        while (*text != '\0')
        {
            if (match_like(text, pattern))
                return 1;

            text++;
        }

        return 0;
    }

    if (*text == '\0')
        return 0;

    if (*pattern == '_' || *pattern == *text)
        return match_like(text + 1, pattern + 1);

    return 0;
}

/* Case-insensitive comparison for SQL column names and keywords. */
static int trm_equals_ignore_case(const char *left, const char *right)
{
    if (left == NULL || right == NULL)
        return 0;

    while (*left != '\0' && *right != '\0')
    {
        if (tolower((unsigned char)*left) !=
            tolower((unsigned char)*right))
            return 0;

        left++;
        right++;
    }

    return *left == '\0' && *right == '\0';
}

/* Evaluate a condition against an entire record. */
static int evaluate_condition(
    const TRM_Record *record,
    const SQLCondition *condition)
{
    if (record == NULL || condition == NULL)
        return 0;

    const char *record_value;

    /* The parser preserves the user's original letter case. */
    if (trm_equals_ignore_case(condition->column, "ID"))
        record_value = record->key;
    else if (trm_equals_ignore_case(condition->column, "VALUE"))
        record_value = record->value;
    else
        return 0;

    if (strcmp(condition->comparison, "=") == 0)
        return compare_values(record_value, condition->value) == 0;

    if (strcmp(condition->comparison, "!=") == 0)
        return compare_values(record_value, condition->value) != 0;

    if (strcmp(condition->comparison, ">") == 0)
        return compare_values(record_value, condition->value) > 0;

    if (strcmp(condition->comparison, "<") == 0)
        return compare_values(record_value, condition->value) < 0;

    if (trm_equals_ignore_case(condition->comparison, "LIKE"))
        return match_like(record_value, condition->value);

    if (trm_equals_ignore_case(condition->comparison, "IN"))
    {
        for (int i = 0; i < condition->value_count; i++)
        {
            if (compare_values(
                    record_value,
                    condition->values[i]) == 0)
                return 1;
        }

        return 0;
    }

    if (trm_equals_ignore_case(condition->comparison, "BETWEEN"))
    {
        int lower = compare_values(record_value, condition->value);
        int upper = compare_values(record_value, condition->second_value);

        return lower >= 0 && upper <= 0;
    }

    return 0;
}

typedef struct
{
    const SQLExpressionToken *tokens;
    int count;
    int position;
    const TRM_Record *record;
    int error;
} TRM_ExpressionParser;

static int trm_parse_or(TRM_ExpressionParser *parser);

static int trm_parse_primary(TRM_ExpressionParser *parser)
{
    if (parser->position >= parser->count)
    {
        parser->error = 1;
        return 0;
    }

    const SQLExpressionToken *token =
        &parser->tokens[parser->position];

    if (token->type == EXPR_CONDITION)
    {
        parser->position++;

        return evaluate_condition(
            parser->record,
            &token->condition
        );
    }

    if (token->type == EXPR_OPEN_PAREN)
    {
        parser->position++;

        int result = trm_parse_or(parser);

        if (parser->position >= parser->count ||
            parser->tokens[parser->position].type != EXPR_CLOSE_PAREN)
        {
            parser->error = 1;
            return 0;
        }

        parser->position++;
        return result;
    }

    parser->error = 1;
    return 0;
}

static int trm_parse_and(TRM_ExpressionParser *parser)
{
    int result = trm_parse_primary(parser);

    while (!parser->error &&
           parser->position < parser->count &&
           parser->tokens[parser->position].type == EXPR_AND)
    {
        parser->position++;

        int right = trm_parse_primary(parser);
        result = result && right;
    }

    return result;
}

static int trm_parse_or(TRM_ExpressionParser *parser)
{
    int result = trm_parse_and(parser);

    while (!parser->error &&
           parser->position < parser->count &&
           parser->tokens[parser->position].type == EXPR_OR)
    {
        parser->position++;

        int right = trm_parse_and(parser);
        result = result || right;
    }

    return result;
}

static int trm_evaluate_expression(
    const TRM_Record *record,
    int expression_count,
    const SQLExpressionToken expression[])
{
    TRM_ExpressionParser parser = {
        expression,
        expression_count,
        0,
        record,
        0
    };

    int result = trm_parse_or(&parser);

    if (parser.error || parser.position != parser.count)
        return -1;

    return result;
}

/* Find a record inside a specific table. */
int table_record_manager_find(
    const char *table_name,
    const char *key,
    TRM_Record *result)
{
    if (table_name == NULL || key == NULL)
    {
        return 0;
    }

    int table_id;

    if (!get_table_id(table_name, &table_id))
    {
        return 0;
    }

    for (int i = 0; i < database.record_count; i++)
    {
        TRM_Record *record = &database.records[i];

        if (record->table_id == table_id &&
            strcmp(record->key, key) == 0)
        {
            if (result != NULL)
            {
                *result = *record;
            }

            return 1;
        }
    }

    return 0;
}

/* Update a record inside a specific table. */
int table_record_manager_update(
    const char *table_name,
    const char *key,
    const char *new_value)
{
    if (table_name == NULL ||
        key == NULL ||
        new_value == NULL)
    {
        return 0;
    }

    if (strlen(new_value) >= RM_MAX_VALUE_SIZE)
    {
        printf("TRM Error: Value is too long.\n");
        return 0;
    }

    int table_id;

    if (!get_table_id(table_name, &table_id))
    {
        return 0;
    }

    for (int i = 0; i < database.record_count; i++)
    {
        TRM_Record *record = &database.records[i];

        if (record->table_id == table_id &&
            strcmp(record->key, key) == 0)
        {
            char old_value[RM_MAX_VALUE_SIZE];

            strcpy(old_value, record->value);
            strcpy(record->value, new_value);

            if (!save_records())
            {
                strcpy(record->value, old_value);
                return 0;
            }

            printf("Record updated successfully.\n");
            printf("Table : %s\n", table_name);
            printf("Key   : %s\n", key);

            return 1;
        }
    }

    printf("Record not found.\n");
    return 0;
}

/* Delete a record from a specific table. */
int table_record_manager_delete(
    const char *table_name,
    const char *key)
{
    if (table_name == NULL || key == NULL)
    {
        return 0;
    }

    int table_id;

    if (!get_table_id(table_name, &table_id))
    {
        return 0;
    }

    for (int i = 0; i < database.record_count; i++)
    {
        TRM_Record *record = &database.records[i];

        if (record->table_id == table_id &&
            strcmp(record->key, key) == 0)
        {
            TRM_Record deleted_record = *record;

            for (int j = i; j < database.record_count - 1; j++)
            {
                database.records[j] = database.records[j + 1];
            }

            database.record_count--;

            memset(
                &database.records[database.record_count],
                0,
                sizeof(TRM_Record));

            if (!save_records())
            {
                for (int j = database.record_count; j > i; j--)
                {
                    database.records[j] = database.records[j - 1];
                }

                database.records[i] = deleted_record;
                database.record_count++;

                return 0;
            }

            printf("Record deleted successfully.\n");
            printf("Table : %s\n", table_name);
            printf("Key   : %s\n", key);

            return 1;
        }
    }

    printf("Record not found.\n");
    return 0;
}

/* List records belonging to one table. */
int table_record_manager_list(const char *table_name)
{
    if (table_name == NULL)
    {
        return 0;
    }

    int table_id;

    if (!get_table_id(table_name, &table_id))
    {
        return 0;
    }

    printf("\nTable: %s\n", table_name);
    printf("========================================\n");

    int found = 0;

    for (int i = 0; i < database.record_count; i++)
    {
        TRM_Record *record = &database.records[i];

        if (record->table_id == table_id)
        {
            printf("Key   : %s\n", record->key);
            printf("Value : %s\n", record->value);
            printf("----------------------------------------\n");

            found++;
        }
    }

    printf("Total records: %d\n", found);

    return found;
}



int table_record_manager_select(
    const char *table_name,
    int show_key,
    int show_value,
    int expression_count,
    const SQLExpressionToken expression[])
{
    if (table_name == NULL ||
        (!show_key && !show_value) ||
        expression_count < 0 ||
        expression_count > SQL_MAX_EXPRESSION_TOKENS ||
        (expression_count > 0 && expression == NULL))
    {
        printf("TRM Error: Invalid SELECT arguments.\n");
        return 0;
    }

    int table_id;

    if (!get_table_id(table_name, &table_id))
    {
        printf("TRM Error: Table not found: %s\n", table_name);
        return 0;
    }

    int found = 0;

    printf("\nTable: %s\n", table_name);
    printf("----------------------------------------\n");

    if (show_key && show_value)
        printf("%-20s | %-30s\n", "ID", "VALUE");
    else if (show_key)
        printf("%-20s\n", "ID");
    else
        printf("%-30s\n", "VALUE");

    printf("----------------------------------------\n");

    for (int i = 0; i < database.record_count; i++)
    {
        TRM_Record *record = &database.records[i];

        if (record->table_id != table_id)
            continue;

        int matches = 1;

        if (expression_count > 0)
        {
            matches = trm_evaluate_expression(
    record,
    expression_count,
    expression
);

            if (matches == -1)
            {
                printf("TRM Error: Invalid WHERE expression.\n");
                return 0;
            }
        }

        if (!matches)
            continue;

        if (show_key && show_value)
            printf("%-20s | %-30s\n",
                   record->key, record->value);
        else if (show_key)
            printf("%-20s\n", record->key);
        else
            printf("%-30s\n", record->value);

        found++;
    }

    printf("----------------------------------------\n");
    printf("Total records: %d\n", found);

    return 1;
}
