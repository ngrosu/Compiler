cmake_minimum_required(VERSION 3.26)
project(Compiler C)

set(CMAKE_C_STANDARD 17)

add_executable(Compiler main.c
        Lexer/lexer.c
        Lexer/lexer.h
        Lexer/dfa_table.c
        Lexer/dfa_table.h
        definitions.h
        Shared/token.h
        Shared/token.c
        Shared/error.c
        Shared/error.h
        Parser/ParserGenerator/DataStructures.c
        Parser/ParserGenerator/DataStructures.h
        Parser/ParserGenerator/Functions.c
        Parser/ParserGenerator/Functions.h
        Parser/ParserGenerator/symbol.c
        Parser/ParserGenerator/symbol.h
)
