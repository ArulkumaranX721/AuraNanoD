
#include <stdio.h>
#include "SQLParser.h"

int main(void)
{
    const char *queries[] =
    {
        "CREATE TABLE employees;",
        "INSERT INTO employees VALUES ('arul001', 'Arul');",
        "SELECT * FROM employees;"
    };

    int query_count = sizeof(queries) / sizeof(queries[0]);

    for (int i = 0; i < query_count; i++)
    {
        SQLTokenList tokens;
        SQLStatement statement;

        printf("\nSQL QUERY: %s\n", queries[i]);

        if (!sql_tokenize(queries[i], &tokens))
        {
            printf("TOKENIZATION FAILED\n");
            continue;
        }

        if (!sql_parse(&tokens, &statement))
        {
            printf("PARSING FAILED\n");
            continue;
        }

        printf("PARSING SUCCESSFUL\n");

        printf("Table Name : %s\n", statement.table_name);

        if (statement.type == SQL_COMMAND_CREATE_TABLE)
        {
            printf("Command    : CREATE TABLE\n");
        }
        else if (statement.type == SQL_COMMAND_INSERT)
        {
            printf("Command    : INSERT\n");
            printf("Key        : %s\n", statement.key);
            printf("Value      : %s\n", statement.value);
        }
        else if (statement.type == SQL_COMMAND_SELECT)
        {
            printf("Command    : SELECT\n");
        }
    }

    return 0;
}
