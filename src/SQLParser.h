#ifndef SQLPARSER_H
#define SQLPARSER_H

#define SQL_MAX_TOKENS 64
#define SQL_MAX_TOKEN_LENGTH 128

typedef enum
{
    SQL_TOKEN_KEYWORD,
    SQL_TOKEN_IDENTIFIER,
    SQL_TOKEN_STRING,
    SQL_TOKEN_NUMBER,
    SQL_TOKEN_SYMBOL
} SQLTokenType;

typedef struct
{
    SQLTokenType type;
    char value[SQL_MAX_TOKEN_LENGTH];
} SQLToken;

typedef struct
{
    SQLToken tokens[SQL_MAX_TOKENS];
    int count;
} SQLTokenList;

int sql_tokenize(const char *sql, SQLTokenList *result);

void sql_print_tokens(const SQLTokenList *tokens);

#endif
