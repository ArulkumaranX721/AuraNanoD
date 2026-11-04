
#include <stdio.h>
#include "SQLParser.h"

int main(void)
{
    const char *sql =
        "INSERT INTO employees VALUES ('arul001', 'Arul');";

    SQLTokenList tokens;

    printf("LuraXDB SQL TOKENIZER TEST\n");
    printf("=========================\n");

    if (!sql_tokenize(sql, &tokens))
    {
        printf("Tokenizer failed.\n");
        return 1;
    }

    sql_print_tokens(&tokens);

    printf("\nTokenizer test completed.\n");

    return 0;
}
