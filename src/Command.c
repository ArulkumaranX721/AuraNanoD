#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "Database.h"
#include "storage.h"
#include "RecordManager.h"
#include "PageManager.h"
#include <stdlib.h>
#include "BufferPool.h"
#include "SQLParser.h"
#include "TableRecordManager.h"
#include "TableManager.h"


void show_help(void)
{
    printf("\nAvailable Commands:\n");
    printf("-------------------\n");
    printf("HELP   - Display available commands\n");
    printf("STATUS - Display database status\n");
    printf("VERSION - Display database version\n");
    printf("EXIT   - Close the database CLI\n");
    printf("CONFIG - Display database configuration\n");
    printf("STORAGE - Display Storage Information\n");
    printf("PUT <key> <value> - Store a record\n");
    printf("UPDATE <key> <value> - Update an existing record\n");
printf("GET <key>     - Retrieve a record\n");
printf("DELETE <key>  - Delete a record\n");
printf("LIST          - List all records\n");
printf("PAGEINFO      - INFORMATION about the PAGE\n");
printf("PAGEDETAIL    - INFORMATION about the PAGEDETAILS\n");
printf("PAGEID        - INFORMATION about the PAGEID\n");
printf("BPOOLINFO - Display buffer pool statistics\n");
printf("BPWRITE <id> <text> - Write text through the buffer pool\n");
printf("BPREAD <id> - Read text through the buffer pool\n");
printf("BPRESET - Reset cache statistics\n");
printf("\nSQL Commands:\n");
printf("CREATE TABLE <name> - Create a table\n");
printf("INSERT INTO <table> VALUES ('key', 'value')\n");
printf("SELECT * FROM <table> - Display table records\n");
printf("UPDATE <table> SET value = 'new value' WHERE id = 'key';\n");
printf("DELETE FROM <table> WHERE id = 'key';\n");
printf("SELECT * FROM <table> ORDER BY id ASC LIMIT 5;\n");
printf("SELECT id, COUNT(*) FROM <table> GROUP BY id;\n");
printf("SELECT value, COUNT(*) FROM <table> GROUP BY value;\n");
printf("SELECT value, AVG(id) FROM <table> GROUP BY value;\n");
printf("SELECT value, MIN(id) FROM <table> GROUP BY value ORDER BY value ASC;\n");
printf("SELECT value, MAX(id) FROM <table> GROUP BY value ORDER BY value DESC;\n");
printf("\nSQL WHERE Features:\n");
printf("WHERE id = '101'\n");
printf("WHERE value = 'Database Engineer'\n");
printf("WHERE value LIKE '%%Database%%'\n");
printf("WHERE id IN ('101', '102')\n");
printf("WHERE id BETWEEN '101' AND '200'\n");
printf("\nSQL WHERE Features:\n");
printf("WHERE id = '101'\n");
printf("WHERE value = 'Database Engineer'\n");
printf("WHERE value LIKE '%%Database%%'\n");
printf("WHERE (id = '101' OR id = '102') AND value LIKE '%%Engineer%%'\n");
printf("WHERE id IN ('101', '102')\n");
printf("WHERE id BETWEEN '101' AND '200'\n");
}

void clear_screen()
{
    system("cls");
}

void convert_to_uppercase(char *text)
{
    while (*text != '\0')
    {
        *text = (char)toupper((unsigned char)*text);
        text++;
    }
}

void show_status(const DatabaseConfig *database)
{
    printf("\nDatabase Status\n");
    printf("-------------------\n");

    if (database->initialized)
    {
        printf("Status : CONFIGURED\n");
    }
    else
    {
        printf("Status : NOT CONFIGURED\n");
    }

    database_show_config(database);
}

void show_version(void)
{
    printf("\nLuraXDB Database Engine\n");
    printf("Version: 0.1.0\n");
    printf("Language: C17\n");
}



void execute_sql_command(const char *sql)
{
    SQLTokenList tokens;
    SQLStatement statement;

    /* Step 1: Tokenize SQL */
    if (!sql_tokenize(sql, &tokens))
    {
        return;
    }

    /* Step 2: Parse SQL */
    if (!sql_parse(&tokens, &statement))
    {
        return;
    }

    /* Step 3: Execute SQL */
    switch (statement.type)
    {
        case SQL_COMMAND_CREATE_TABLE:

            table_manager_create(statement.table_name);

            break;

        case SQL_COMMAND_INSERT:

            table_record_manager_insert(
                statement.table_name,
                statement.key,
                statement.value
            );

            break;


    case SQL_COMMAND_SELECT:

    if (statement.has_group_by)
    {
        table_record_manager_group_by(
            statement.table_name,
            statement.group_by_column,
            statement.aggregate_type,
            statement.aggregate_column,
            statement.expression_count,
            statement.expression,
            statement.has_order_by ? statement.order_by_column : NULL,
            statement.order_desc
        );
    }
    else if (statement.aggregate_type != SQL_AGGREGATE_NONE)
    {
        if (statement.aggregate_type == SQL_AGGREGATE_COUNT)
        {
            table_record_manager_count(
                statement.table_name,
                statement.aggregate_column,
                statement.expression_count,
                statement.expression
            );
        }
        else
        {
            table_record_manager_aggregate(
                statement.table_name,
                statement.aggregate_type,
                statement.aggregate_column,
                statement.expression_count,
                statement.expression
            );
        }
    }
    else
    {
        table_record_manager_select(
    statement.table_name,
    statement.select_key,
    statement.select_value,
    statement.expression_count,
    statement.expression,
    statement.has_order_by ? statement.order_by_column : NULL,
    statement.order_desc,
    statement.has_limit,
    statement.limit
        );
    }

    break;

   case SQL_COMMAND_UPDATE:
    table_record_manager_update_where(
        statement.table_name,
        statement.value,
        statement.expression_count,
        statement.expression
    );
    break;

   case SQL_COMMAND_DELETE:
    table_record_manager_delete_where(
        statement.table_name,
        statement.expression_count,
        statement.expression
    );
    break;

        default:

            printf("SQL Executor: Unsupported SQL statement.\n");

            break;
    }
}


   void command_loop(DatabaseConfig *database)
{
    char command[512];
    char operation[32];

    printf("\nLuraXDB Command Line Interface\n");
    printf("Type HELP to see available commands.\n");

    while (1)
    {
        printf("\nLuraXDB> ");

        if (fgets(command, sizeof(command), stdin) == NULL)
        {
            printf("\nInput closed. Exiting CLI.\n");
            break;
        }

        if (strchr(command, '\n') == NULL &&
            !feof(stdin))
        {
            int ch;

            while ((ch = getchar()) != '\n' && ch != EOF)
            {
                /* Discard the remainder of an overlong line. */
            }

            printf("Error: Command is too long.\n");
            continue;
        }

        command[strcspn(command, "\r\n")] = '\0';

        char *cursor = command;

        while (isspace((unsigned char)*cursor))
        {
            cursor++;
        }

        if (*cursor == '\0')
        {
            continue;
        }

        char *operation_start = cursor;

        while (*cursor != '\0' &&
               !isspace((unsigned char)*cursor))
        {
            cursor++;
        }

        size_t operation_length =
            (size_t)(cursor - operation_start);

        if (operation_length >= sizeof(operation))
        {
            printf("Error: Command name is too long.\n");
            continue;
        }

        memcpy(operation, operation_start, operation_length);
        operation[operation_length] = '\0';

        for (size_t i = 0; operation[i] != '\0'; i++)
        {
            operation[i] =
                (char)toupper((unsigned char)operation[i]);
        }

        while (isspace((unsigned char)*cursor))
        {
            cursor++;
        }

        char *arguments = cursor;

        if (strcmp(operation, "CREATE") == 0 ||
    strcmp(operation, "INSERT") == 0 ||
    strcmp(operation, "SELECT") == 0 ||
    strcmp(operation, "UPDATE") == 0 ||
    strcmp(operation, "DELETE") == 0)
{
    execute_sql_command(command);
}
else if (strcmp(operation, "HELP") == 0)
{
    show_help();
}
        else if (strcmp(operation, "STATUS") == 0)
        {
            show_status(database);
        }
        else if (strcmp(operation, "VERSION") == 0)
        {
            show_version();
        }
        else if (strcmp(operation, "CONFIG") == 0)
        {
            database_show_config(database);
        }
        else if (strcmp(operation, "BPOOLINFO") == 0)
{
    buffer_pool_show_info();
}
else if (strcmp(operation, "BPWRITE") == 0)
{
    int page_id;
    int consumed = 0;
    char *text;

    if (sscanf(arguments, "%d%n", &page_id, &consumed) != 1)
    {
        printf("Usage: BPWRITE <id> <text>\n");
        continue;
    }

    text = arguments + consumed;

    while (isspace((unsigned char)*text))
    {
        text++;
    }

    if (*text == '\0')
    {
        printf("Usage: BPWRITE <id> <text>\n");
        continue;
    }

    buffer_pool_write_text(page_id, text);
}
else if (strcmp(operation, "BPREAD") == 0)
{
    int page_id;
    int consumed = 0;
    char buffer[PAGE_SIZE];

    if (sscanf(arguments, "%d%n", &page_id, &consumed) != 1)
    {
        printf("Usage: BPREAD <id>\n");
        continue;
    }

    while (isspace((unsigned char)arguments[consumed]))
    {
        consumed++;
    }

    if (arguments[consumed] != '\0')
    {
        printf("Usage: BPREAD <id>\n");
        continue;
    }

    if (buffer_pool_read_text(page_id, buffer, sizeof(buffer)))
    {
        printf("Value: %s\n", buffer);
    }
}
else if (strcmp(operation, "BPRESET") == 0)
{
    buffer_pool_reset_stats();
}
        else if (strcmp(operation, "STORAGE") == 0)
        {
            storage_show_info(database);
        }
       else if (strcmp(operation, "PUT") == 0)
{
    char key[RM_MAX_KEY_SIZE];
    char *value_start = arguments;

    size_t key_length = 0;

    while (value_start[key_length] != '\0' &&
           !isspace((unsigned char)value_start[key_length]))
    {
        key_length++;
    }

    if (key_length == 0 || key_length >= sizeof(key))
    {
        printf("Usage: PUT <key> <value>\n");
        continue;
    }

    memcpy(key, value_start, key_length);
    key[key_length] = '\0';

    value_start += key_length;

    while (isspace((unsigned char)*value_start))
    {
        value_start++;
    }

    if (*value_start == '\0')
    {
        printf("Usage: PUT <key> <value>\n");
        continue;
    }

    record_manager_insert(key, value_start);
}
else if (strcmp(operation, "UPDATE") == 0)
{
    char key[RM_MAX_KEY_SIZE];
    char *value_start = arguments;

    size_t key_length = 0;

    while (value_start[key_length] != '\0' &&
           !isspace((unsigned char)value_start[key_length]))
    {
        key_length++;
    }

    if (key_length == 0 || key_length >= sizeof(key))
    {
        printf("Usage: UPDATE <key> <new_value>\n");
        continue;
    }

    memcpy(key, value_start, key_length);
    key[key_length] = '\0';

    value_start += key_length;

    while (isspace((unsigned char)*value_start))
    {
        value_start++;
    }

    if (*value_start == '\0')
    {
        printf("Usage: UPDATE <key> <new_value>\n");
        continue;
    }

    record_manager_update(key, value_start);
}
        else if (strcmp(operation, "GET") == 0)
{
    char key[RM_MAX_KEY_SIZE];
    RM_Record result;

    if (sscanf(arguments, "%63s", key) != 1)
    {
        printf("Usage: GET <key>\n");
        continue;
    }

    if (record_manager_find(key, &result))
    {
        printf("\nRecord Found\n");
        printf("Key   : %s\n", result.key);
        printf("Value : %s\n", result.value);
    }
    else
    {
        printf("Error: Key not found.\n");
    }
}
       else if (strcmp(operation, "DELETE") == 0)
{
    char key[RM_MAX_KEY_SIZE];

    if (sscanf(arguments, "%63s", key) != 1)
    {
        printf("Usage: DELETE <key>\n");
        continue;
    }

    record_manager_delete(key);
}
        else if (strcmp(operation, "LIST") == 0)
{
    record_manager_list();
}
        else if (strcmp(operation, "EXIT") == 0)
        {
            printf("Shutting down LuraXDB CLI...\n");
            break;
        }
        else if(strcmp(operation,"CLS")==0 || strcmp(operation,"CLEAR")==0 || strcmp(operation,"CLEARSCREEN")==0)
        {
            clear_screen();
        }
        else if (strcmp(operation, "PAGEINFO") == 0)
{
    page_manager_show_info();
}
else if (strcmp(operation, "PAGEWRITE") == 0)
{
    int page_id;
    int consumed = 0;
    char *text;

    if (sscanf(arguments, "%d%n", &page_id, &consumed) != 1)
    {
        printf("Usage: PAGEWRITE <id> <text>\n");
        continue;
    }

    text = arguments + consumed;

    while (isspace((unsigned char)*text))
    {
        text++;
    }

    if (*text == '\0')
    {
        printf("Usage: PAGEWRITE <id> <text>\n");
        continue;
    }

    page_manager_write_text(page_id, text);
}
else if (strcmp(operation, "PAGEREAD") == 0)
{
    int page_id;
    int consumed = 0;
    char buffer[PAGE_SIZE];

    if (sscanf(arguments, "%d%n", &page_id, &consumed) != 1)
    {
        printf("Usage: PAGEREAD <id>\n");
        continue;
    }

    while (isspace((unsigned char)arguments[consumed]))
    {
        consumed++;
    }

    if (arguments[consumed] != '\0')
    {
        printf("Usage: PAGEREAD <id>\n");
        continue;
    }

    if (page_manager_read_text(page_id, buffer, sizeof(buffer)))
    {
        printf("Page %d contents: %s\n", page_id, buffer);
    }
}
        else
        {
            printf("Unknown command: %s\n", operation);
            printf("Type HELP for available commands.\n");
        }
    }
}
     
