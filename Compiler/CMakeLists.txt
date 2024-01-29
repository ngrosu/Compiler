cmake_minimum_required(VERSION 3.26)
project(Compiler C)

set(CMAKE_C_STANDARD 17)

add_executable(Compiler main.c
        Lexer/lexer.c
        Lexer/lexer.h
        Lexer/dfa_table.c
        Lexer/dfa_table.h
)
