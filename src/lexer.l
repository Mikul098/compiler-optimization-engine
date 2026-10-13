/*
 * ==============================================================================
 * File: src/lexer.l
 * Purpose: Lexical Analyzer (Scanner) specification for Flex.
 *
 * Role in Compiler:
 *  - Front-end phase: converts raw input source text (stream of characters) into
 *    a linear stream of meaningful tokens for the Bison parser.
 *  - Strips out whitespace, newlines, carriage returns, and single-line comments.
 *  - Identifies language keywords (e.g., 'print'), arithmetic operators
 *    ('+', '-', '*', '/'), assignment ('='), delimiters ('(', ')', ';'),
 *    identifiers (variable names), and numeric integer literals.
 *  - Tracks line numbers via 'yylineno' for accurate compile-time error reporting.
 * ==============================================================================
 */

%{
#include <cstdio>
#if defined(_WIN32) && !defined(fileno)
#define fileno _fileno
#endif

#include "ast.h"
#include "parser.tab.hpp"
#include <iostream>
#include <string>
#include <cstdlib>
%}

%option noyywrap
%option yylineno
%option nounput
%option noinput
%option never-interactive

%%

"//".*                  { /* ignore single-line comments */ }
[ \t\n\r]+              { /* ignore whitespace */ }

"print"                 { return PRINT; }

"+"                     { return PLUS; }
"-"                     { return MINUS; }
"*"                     { return MUL; }
"/"                     { return DIV; }
"="                     { return ASSIGN; }
"("                     { return LPAREN; }
")"                     { return RPAREN; }
";"                     { return SEMI; }

[a-zA-Z_][a-zA-Z0-9_]*  {
    // Allocate string for Bison semantic value
    yylval.str_val = new std::string(yytext);
    return IDENTIFIER;
}

[0-9]+                  {
    // Pass integer value to Bison
    yylval.int_val = std::atoi(yytext);
    return NUMBER;
}

.                       {
    std::cerr << "Lexical error: Unrecognized character '" << yytext 
              << "' on line " << yylineno << std::endl;
}

%%
