
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "SQLParser.h"

/* Check whether a word is an SQL keyword. */
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

/* Add a token to the token list. */
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

    if (length < 0 || length >= SQL_MAX_TOKEN_LENGTH)
    {
        printf("SQL Parser Error: Token is too long.\n");
        return 0;
    }

    SQLToken *token = &result->tokens[result->count];

    token->type = type;

    memcpy(token->value, value, (size_t)length);
    token->value[length] = '\0';

    result->count++;

    return 1;
}

/* SQL TOKENIZER */
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
        /* Ignore whitespace. */
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

     /* SQL symbols. */

/* SQL symbols */

/* Handle two-character operator: != */
if (sql[i] == '!')
{
    if (sql[i + 1] == '=')
    {
        if (!add_token(result, SQL_TOKEN_SYMBOL, &sql[i], 2))
        {
            return 0;
        }

        i += 2;
        continue;
    }

    printf("SQL Tokenizer Error: Invalid operator '!'.\n");
    return 0;
}

/* Handle single-character symbols */
if (strchr("(),;*=<>", sql[i]) != NULL)
{
    if (!add_token(result, SQL_TOKEN_SYMBOL, &sql[i], 1))
    {
        return 0;
    }

    i++;
    continue;
}

        /* Numbers. */
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

        /* Identifiers and keywords. */
        if (isalpha((unsigned char)sql[i]) || sql[i] == '_')
        {
            char word[SQL_MAX_TOKEN_LENGTH];
            char upper_word[SQL_MAX_TOKEN_LENGTH];

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

                /* Preserve original spelling. */
                word[length++] = sql[i];

                i++;
            }

            word[length] = '\0';

            /* Uppercase copy for keyword detection. */
            for (int j = 0; j <= length; j++)
            {
                upper_word[j] = (char)toupper(
                    (unsigned char)word[j]);
            }

            SQLTokenType type;

            if (is_sql_keyword(upper_word))
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
                    word,
                    i - start))
            {
                return 0;
            }

            /* Normalize keywords only. */
            if (type == SQL_TOKEN_KEYWORD)
            {
                strcpy(
                    result->tokens[result->count - 1].value,
                    upper_word);
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

/* DISPLAY TOKENIZER OUTPUT */
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

/* CASE-INSENSITIVE TOKEN COMPARISON */
static int is_token(
    const SQLTokenList *tokens,
    int position,
    const char *expected)
{
    if (tokens == NULL || expected == NULL)
    {
        return 0;
    }

    if (position < 0 || position >= tokens->count)
    {
        return 0;
    }

    const char *actual = tokens->tokens[position].value;

    while (*actual != '\0' && *expected != '\0')
    {
        if (toupper((unsigned char)*actual) !=
            toupper((unsigned char)*expected))
        {
            return 0;
        }

        actual++;
        expected++;
    }

    return *actual == '\0' && *expected == '\0';
}

/* SQL PARSER */
int sql_parse(
    const SQLTokenList *tokens,
    SQLStatement *statement)
{
    if (tokens == NULL || statement == NULL)
    {
        return 0;
    }

    memset(statement, 0, sizeof(SQLStatement));

    /* CREATE TABLE employees; */
    if (is_token(tokens, 0, "CREATE"))
    {
        if (tokens->count < 3 ||
            !is_token(tokens, 1, "TABLE") ||
            tokens->tokens[2].type != SQL_TOKEN_IDENTIFIER)
        {
            printf("SQL Parser Error: Invalid CREATE TABLE syntax.\n");
            return 0;
        }

        int end = 3;

        if (end < tokens->count && is_token(tokens, end, ";"))
        {
            end++;
        }

        if (end != tokens->count)
        {
            printf("SQL Parser Error: Unexpected CREATE TABLE tokens.\n");
            return 0;
        }

        statement->type = SQL_COMMAND_CREATE_TABLE;

        strcpy(
            statement->table_name,
            tokens->tokens[2].value);

        return 1;
    }

    /* INSERT INTO employees VALUES ('101', 'Arul'); */
    if (is_token(tokens, 0, "INSERT"))
    {
        if (tokens->count < 9 ||
            !is_token(tokens, 1, "INTO") ||
            tokens->tokens[2].type != SQL_TOKEN_IDENTIFIER ||
            !is_token(tokens, 3, "VALUES") ||
            !is_token(tokens, 4, "(") ||
            tokens->tokens[5].type != SQL_TOKEN_STRING ||
            !is_token(tokens, 6, ",") ||
            tokens->tokens[7].type != SQL_TOKEN_STRING ||
            !is_token(tokens, 8, ")"))
        {
            printf("SQL Parser Error: Invalid INSERT syntax.\n");
            return 0;
        }

        int end = 9;

        if (end < tokens->count && is_token(tokens, end, ";"))
        {
            end++;
        }

        if (end != tokens->count)
        {
            printf("SQL Parser Error: Unexpected INSERT tokens.\n");
            return 0;
        }

        statement->type = SQL_COMMAND_INSERT;

        strcpy(statement->table_name, tokens->tokens[2].value);
        strcpy(statement->key, tokens->tokens[5].value);
        strcpy(statement->value, tokens->tokens[7].value);

        return 1;
    }

    /* UPDATE employees SET value = 'Senior Engineer' WHERE id = '101'; */
    if (is_token(tokens, 0, "UPDATE"))
    {
        if (tokens->count < 10 ||
            tokens->tokens[1].type != SQL_TOKEN_IDENTIFIER ||
            !is_token(tokens, 2, "SET") ||
            !is_token(tokens, 3, "VALUE") ||
            !is_token(tokens, 4, "=") ||
            tokens->tokens[5].type != SQL_TOKEN_STRING ||
            !is_token(tokens, 6, "WHERE") ||
            !is_token(tokens, 7, "ID") ||
            !is_token(tokens, 8, "=") ||
            tokens->tokens[9].type != SQL_TOKEN_STRING)
        {
            printf("SQL Parser Error: Invalid UPDATE syntax.\n");
            return 0;
        }

        int end = 10;

        if (end < tokens->count && is_token(tokens, end, ";"))
        {
            end++;
        }

        if (end != tokens->count)
        {
            printf("SQL Parser Error: Unexpected UPDATE tokens.\n");
            return 0;
        }

        statement->type = SQL_COMMAND_UPDATE;

        strcpy(statement->table_name, tokens->tokens[1].value);
        strcpy(statement->value, tokens->tokens[5].value);
        strcpy(statement->key, tokens->tokens[9].value);

        return 1;
    }

    /* DELETE FROM employees WHERE id = '101'; */
    if (is_token(tokens, 0, "DELETE"))
    {
        if (tokens->count < 7 ||
            !is_token(tokens, 1, "FROM") ||
            tokens->tokens[2].type != SQL_TOKEN_IDENTIFIER ||
            !is_token(tokens, 3, "WHERE") ||
            !is_token(tokens, 4, "ID") ||
            !is_token(tokens, 5, "=") ||
            tokens->tokens[6].type != SQL_TOKEN_STRING)
        {
            printf("SQL Parser Error: Invalid DELETE syntax.\n");
            return 0;
        }

        int end = 7;

        if (end < tokens->count && is_token(tokens, end, ";"))
        {
            end++;
        }

        if (end != tokens->count)
        {
            printf("SQL Parser Error: Unexpected DELETE tokens.\n");
            return 0;
        }

        statement->type = SQL_COMMAND_DELETE;

        strcpy(statement->table_name, tokens->tokens[2].value);
        strcpy(statement->key, tokens->tokens[6].value);

        return 1;
    }

    /* SELECT * FROM employees; */
   
/*
    Supported SELECT statements:

    SELECT * FROM employees;
    SELECT id FROM employees;
    SELECT value FROM employees;
    SELECT id, value FROM employees;
    SELECT id, value FROM employees WHERE id = '101';
*/

if (is_token(tokens, 0, "SELECT"))
{
    int pos = 1;

    statement->type = SQL_COMMAND_SELECT;
    statement->has_where = 0;
    statement->select_key = 0;
    statement->select_value = 0;
    statement->has_second_condition = 0;
statement->has_or_condition = 0;

    /* SELECT * */
    if (is_token(tokens, pos, "*"))
    {
        statement->select_key = 1;
        statement->select_value = 1;
        pos++;
    }
    else
    {
        /* First selected column */
        if (is_token(tokens, pos, "ID"))
        {
            statement->select_key = 1;
        }
        else if (is_token(tokens, pos, "VALUE"))
        {
            statement->select_value = 1;
        }
        else
        {
            printf("SQL Parser Error: Invalid SELECT column.\n");
            return 0;
        }

        pos++;

        /* Optional second column */
        if (is_token(tokens, pos, ","))
        {
            pos++;

            if (is_token(tokens, pos, "ID"))
            {
                if (statement->select_key)
                {
                    printf("SQL Parser Error: Duplicate column.\n");
                    return 0;
                }

                statement->select_key = 1;
            }
            else if (is_token(tokens, pos, "VALUE"))
            {
                if (statement->select_value)
                {
                    printf("SQL Parser Error: Duplicate column.\n");
                    return 0;
                }

                statement->select_value = 1;
            }
            else
            {
                printf("SQL Parser Error: Invalid SELECT column.\n");
                return 0;
            }

            pos++;
        }
    }

    /* FROM table */
    if (!is_token(tokens, pos, "FROM") ||
        pos + 1 >= tokens->count ||
        tokens->tokens[pos + 1].type != SQL_TOKEN_IDENTIFIER)
    {
        printf("SQL Parser Error: Invalid SELECT FROM syntax.\n");
        return 0;
    }

    strcpy(
        statement->table_name,
        tokens->tokens[pos + 1].value
    );

    pos += 2;

    /* Optional WHERE condition */
if (is_token(tokens, pos, "WHERE"))
{
    /* First condition */
    if (pos + 3 >= tokens->count ||
        !is_token(tokens, pos + 1, "ID") ||
        tokens->tokens[pos + 3].type != SQL_TOKEN_STRING)
    {
        printf("SQL Parser Error: Invalid WHERE condition.\n");
        return 0;
    }

    if (!is_token(tokens, pos + 2, "=") &&
        !is_token(tokens, pos + 2, "!=") &&
        !is_token(tokens, pos + 2, ">") &&
        !is_token(tokens, pos + 2, "<"))
    {
        printf("SQL Parser Error: Invalid WHERE operator.\n");
        return 0;
    }

    strcpy(statement->key, tokens->tokens[pos + 3].value);

    strcpy(statement->condition_operator,
           tokens->tokens[pos + 2].value);

    statement->has_where = 1;
    statement->has_second_condition = 0;

    pos += 4;

    /* Optional AND condition */
/* Optional AND / OR condition */
if (is_token(tokens, pos, "AND") ||
    is_token(tokens, pos, "OR"))
{
    int is_or = is_token(tokens, pos, "OR");

    pos++;

    if (pos + 2 >= tokens->count ||
        !is_token(tokens, pos, "ID") ||
        tokens->tokens[pos + 2].type != SQL_TOKEN_STRING)
    {
        printf("SQL Parser Error: Invalid second WHERE condition.\n");
        return 0;
    }

    if (!is_token(tokens, pos + 1, "=") &&
        !is_token(tokens, pos + 1, "!=") &&
        !is_token(tokens, pos + 1, ">") &&
        !is_token(tokens, pos + 1, "<"))
    {
        printf("SQL Parser Error: Invalid WHERE operator.\n");
        return 0;
    }

    if (is_or)
    {
        strcpy(statement->or_operator, tokens->tokens[pos + 1].value);
        strcpy(statement->or_key, tokens->tokens[pos + 2].value);
        statement->has_or_condition = 1;
    }
    else
    {
        strcpy(statement->second_operator, tokens->tokens[pos + 1].value);
        strcpy(statement->second_key, tokens->tokens[pos + 2].value);
        statement->has_second_condition = 1;
    }

    pos += 3;
}
else
{
    statement->has_second_condition = 0;
}

    /* Optional semicolon */
    if (pos < tokens->count && is_token(tokens, pos, ";"))
    {
        pos++;
    }

    if (pos != tokens->count)
    {
        printf("SQL Parser Error: Unexpected SELECT tokens.\n");
        return 0;
    }

    return 1;
}
}

      

    printf("SQL Parser Error: Unsupported SQL command.\n");
    return 0;
}

