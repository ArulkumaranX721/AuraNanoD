
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "SQLParser.h"

static int is_sql_keyword(const char *word)
{
    const char *keywords[] =
    {
        "CREATE",
        "TABLE",
        "INSERT",
        "INTO",
        "VALUES",
        "SELECT",
        "FROM",
        "WHERE",
        "UPDATE",
        "SET",
        "DELETE",
        "AND",
        "OR",
        "DROP"
    };

    int count = sizeof(keywords) / sizeof(keywords[0]);

    for (int i = 0; i < count; i++)
    {
        if (strcmp(word, keywords[i]) == 0)
        {
            return 1;
        }
    }

    return 0;
}

static int add_token(
    SQLTokenList *result,
    SQLTokenType type,
    const char *value,
    int length)
{
    if (result->count >= SQL_MAX_TOKENS)
    {
        printf("SQL Parser Error: Too many tokens.\n");
        return 0;
    }

    if (length >= SQL_MAX_TOKEN_LENGTH)
    {
        printf("SQL Parser Error: Token is too long.\n");
        return 0;
    }

    SQLToken *token = &result->tokens[result->count];

    token->type = type;

    memcpy(token->value, value, length);
    token->value[length] = '\0';

    result->count++;

    return 1;
}

int sql_tokenize(const char *sql, SQLTokenList *result)
{
    if (sql == NULL || result == NULL)
    {
        return 0;
    }

    result->count = 0;

    int i = 0;

    while (sql[i] != '\0')
    {
        if (isspace((unsigned char)sql[i]))
        {
            i++;
            continue;
        }

        /* SQL string: 'Arul' */

        if (sql[i] == '\'')
        {
            int start = ++i;

            while (sql[i] != '\0' && sql[i] != '\'')
            {
                i++;
            }

            if (sql[i] == '\0')
            {
                printf("SQL Parser Error: Unterminated string.\n");
                return 0;
            }

            if (!add_token(
                    result,
                    SQL_TOKEN_STRING,
                    &sql[start],
                    i - start))
            {
                return 0;
            }

            i++;
            continue;
        }

        /* SQL symbols */

        if (strchr("(),;*=<>", sql[i]) != NULL)
        {
            if (!add_token(
                    result,
                    SQL_TOKEN_SYMBOL,
                    &sql[i],
                    1))
            {
                return 0;
            }

            i++;
            continue;
        }

        /* Numbers */

        if (isdigit((unsigned char)sql[i]))
        {
            int start = i;

            while (isdigit((unsigned char)sql[i]))
            {
                i++;
            }

            if (!add_token(
                    result,
                    SQL_TOKEN_NUMBER,
                    &sql[start],
                    i - start))
            {
                return 0;
            }

            continue;
        }

        /* Identifiers and keywords */

        if (isalpha((unsigned char)sql[i]) || sql[i] == '_')
        {
            char word[SQL_MAX_TOKEN_LENGTH];
            int start = i;
            int length = 0;

            while (
                isalnum((unsigned char)sql[i]) ||
                sql[i] == '_')
            {
                if (length >= SQL_MAX_TOKEN_LENGTH - 1)
                {
                    printf("SQL Parser Error: Identifier too long.\n");
                    return 0;
                }

                word[length++] = (char)toupper(
                    (unsigned char)sql[i]);

                i++;
            }

            word[length] = '\0';

            SQLTokenType type;

            if (is_sql_keyword(word))
            {
                type = SQL_TOKEN_KEYWORD;
            }
            else
            {
                type = SQL_TOKEN_IDENTIFIER;
            }

            if (!add_token(
                    result,
                    type,
                    &sql[start],
                    i - start))
            {
                return 0;
            }

            /* Normalize keywords to uppercase. */

            if (type == SQL_TOKEN_KEYWORD)
            {
                strcpy(
                    result->tokens[result->count - 1].value,
                    word);
            }

            continue;
        }

        printf(
            "SQL Parser Error: Invalid character '%c'.\n",
            sql[i]);

        return 0;
    }

    return 1;
}

void sql_print_tokens(const SQLTokenList *tokens)
{
    if (tokens == NULL)
    {
        return;
    }

    printf("\nSQL Tokenizer Output\n");
    printf("--------------------\n");

    for (int i = 0; i < tokens->count; i++)
    {
        const char *type_name;

        switch (tokens->tokens[i].type)
        {
            case SQL_TOKEN_KEYWORD:
                type_name = "KEYWORD";
                break;

            case SQL_TOKEN_IDENTIFIER:
                type_name = "IDENTIFIER";
                break;

            case SQL_TOKEN_STRING:
                type_name = "STRING";
                break;

            case SQL_TOKEN_NUMBER:
                type_name = "NUMBER";
                break;

            case SQL_TOKEN_SYMBOL:
                type_name = "SYMBOL";
                break;

            default:
                type_name = "UNKNOWN";
                break;
        }

        printf(
            "%-12s : %s\n",
            type_name,
            tokens->tokens[i].value);
    }
}
