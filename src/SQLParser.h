
#ifndef SQLPARSER_H
#define SQLPARSER_H

#define SQL_MAX_TOKENS 64
#define SQL_MAX_TOKEN_LENGTH 128

/* SQL token types */
typedef enum
{
    SQL_TOKEN_KEYWORD,
    SQL_TOKEN_IDENTIFIER,
    SQL_TOKEN_STRING,
    SQL_TOKEN_NUMBER,
    SQL_TOKEN_SYMBOL
} SQLTokenType;

/* Individual SQL token */
typedef struct
{
    SQLTokenType type;
    char value[SQL_MAX_TOKEN_LENGTH];
} SQLToken;

/* Collection of SQL tokens */
typedef struct
{
    SQLToken tokens[SQL_MAX_TOKENS];
    int count;
} SQLTokenList;

/* SQL tokenizer */
int sql_tokenize(const char *sql, SQLTokenList *result);

/* Display tokens */
void sql_print_tokens(const SQLTokenList *tokens);

/* Supported SQL commands */
typedef enum
{
    SQL_COMMAND_UNKNOWN,
    SQL_COMMAND_CREATE_TABLE,
    SQL_COMMAND_INSERT,
    SQL_COMMAND_SELECT,
    SQL_COMMAND_UPDATE,
    SQL_COMMAND_DELETE
} SQLCommandType;

/* Parsed SQL statement */
typedef struct
{
    SQLCommandType type;

    char table_name[SQL_MAX_TOKEN_LENGTH];
    char key[SQL_MAX_TOKEN_LENGTH];
    char value[SQL_MAX_TOKEN_LENGTH];
    int has_where;
      int select_key;
    int select_value;
    char condition_operator[3];
    int has_second_condition;

char second_key[SQL_MAX_TOKEN_LENGTH];
char second_operator[3];
int has_or_condition;
char or_key[SQL_MAX_TOKEN_LENGTH];
char or_operator[3];

} SQLStatement;


/* SQL parser */
int sql_parse(
    const SQLTokenList *tokens,
    SQLStatement *statement
);

#endif
 