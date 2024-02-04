#include <stdio.h>
#include "Lexer/dfa_table.h"
#include "Lexer/lexer.h"
#include "Shared/token.h"

int main()
{
    Lexer lexer = init_lexer("../../test.chad");
    tokenize(lexer);
    print_tokens(lexer);
}