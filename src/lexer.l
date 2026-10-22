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

"//".*                                                  { /* ignore single-line comments */ }
"/*"([^*]|\*+[^*/])*\*+"/"                              { /* ignore multi-line comments */ }
"using"[ \t]+"namespace"[ \t]+[a-zA-Z_][a-zA-Z0-9_]*[ \t]*";" { /* ignore using namespace */ }
"using"[ \t]+[^\n;]+";"                                 { /* ignore using aliases */ }
"public:"|"private:"|"protected:"                       { /* ignore access labels */ }
"#"[ \t]*"include"[ \t]*[<"][^>"\n]+[>"]                { /* ignore include preprocessor */ }
"#"[^\n]*                                               { /* ignore generic preprocessor directives */ }
"template"[ \t]*<[^>\n]*>                               { return TYPE_KEYWORD; }
("decltype"|"typeof"|"typeof_unqual")[ \t]*\([^)]*\)    { return TYPE_KEYWORD; }
("alignas"|"_Alignas")[ \t]*\([^)]*\)                   { return TYPE_KEYWORD; }
[ \t\n\r]+                                              { /* ignore whitespace */ }

"std::cout"|"cout"|"std::cerr"|"cerr"|"std::clog"|"clog" { return COUT; }
"std::endl"|"endl"                                      { return ENDL; }
"std::cin"|"cin"                                        { return CIN; }
"printf"                                                { return PRINTF; }
"print"                                                 { return PRINT; }
"return"                                                { return RETURN; }
"main"                                                  { return MAIN; }

"if"                                                    { return IF; }
"else"                                                  { return ELSE; }
"while"                                                 { return WHILE; }
"for"                                                   { return FOR; }
"do"                                                    { return DO; }
"switch"                                                { return SWITCH; }
"case"                                                  { return CASE; }
"default"                                               { return DEFAULT; }
"break"                                                 { return BREAK; }
"continue"                                              { return CONTINUE; }
"goto"                                                  { return GOTO; }
"sizeof"                                                { return SIZEOF; }
"try"                                                   { return TRY; }
"catch"                                                 { return CATCH; }
"throw"                                                 { return THROW; }

"static_cast"                                           { return STATIC_CAST; }
"dynamic_cast"                                          { return DYNAMIC_CAST; }
"const_cast"                                            { return CONST_CAST; }
"reinterpret_cast"                                      { return REINTERPRET_CAST; }
"static_assert"|"_Static_assert"                        { return STATIC_ASSERT; }
"new"                                                   { return NEW; }
"delete"                                                { return DELETE; }
"co_await"                                              { return CO_AWAIT; }
"co_return"                                             { return CO_RETURN; }
"co_yield"                                              { return CO_YIELD; }

"and"                                                   { return AND_OP; }
"or"                                                    { return OR_OP; }
"not"                                                   { return NOT_OP; }
"bitand"                                                { return AMP; }
"bitor"                                                 { return PIPE; }
"xor"                                                   { return CARET; }
"compl"                                                 { return TILDE; }
"and_eq"                                                { return AND_ASSIGN; }
"or_eq"                                                 { return OR_ASSIGN; }
"xor_eq"                                                { return XOR_ASSIGN; }
"not_eq"                                                { return NE; }

"true"|"TRUE"                                           { yylval.int_val = 1; return NUMBER; }
"false"|"FALSE"                                         { yylval.int_val = 0; return NUMBER; }
"NULL"|"nullptr"                                        { yylval.int_val = 0; return NUMBER; }

"__FILE__"                                              { yylval.str_val = new std::string("\"file.c\""); return STRING_LITERAL; }
"__LINE__"                                              { yylval.int_val = yylineno; return NUMBER; }
"__DATE__"                                              { yylval.str_val = new std::string("\"Oct 23 2026\""); return STRING_LITERAL; }
"__TIME__"                                              { yylval.str_val = new std::string("\"00:00:00\""); return STRING_LITERAL; }
"__func__"                                              { yylval.str_val = new std::string("\"main\""); return STRING_LITERAL; }
"__STDC__"                                              { yylval.int_val = 1; return NUMBER; }
"__STDC_VERSION__"                                      { yylval.int_val = 202311; return NUMBER; }
"__cplusplus"                                           { yylval.int_val = 202302; return NUMBER; }
"INT_MAX"                                               { yylval.int_val = 2147483647; return NUMBER; }
"INT_MIN"                                               { yylval.int_val = -2147483647; return NUMBER; }
"CHAR_BIT"                                              { yylval.int_val = 8; return NUMBER; }
"CHAR_MAX"                                              { yylval.int_val = 127; return NUMBER; }
"CHAR_MIN"                                              { yylval.int_val = -128; return NUMBER; }
"RAND_MAX"                                              { yylval.int_val = 32767; return NUMBER; }
"EOF"                                                   { yylval.int_val = -1; return NUMBER; }
"EXIT_SUCCESS"                                          { yylval.int_val = 0; return NUMBER; }
"EXIT_FAILURE"                                          { yylval.int_val = 1; return NUMBER; }

"int"|"float"|"double"|"char"|"long"|"short"|"unsigned"|"signed"|"void"|"auto"|"const"|"var"|"let"|"bool"|"size_t"|"int8_t"|"int16_t"|"int32_t"|"int64_t"|"uint8_t"|"uint16_t"|"uint32_t"|"uint64_t"|"wchar_t"|"char8_t"|"char16_t"|"char32_t"|"_Bool"|"_Complex"|"_Imaginary"|"_BitInt"|"_Decimal32"|"_Decimal64"|"_Decimal128"|"_Atomic"|"struct"|"class"|"union"|"enum"|"typedef"|"typename"|"template"|"namespace"|"static"|"extern"|"inline"|"virtual"|"explicit"|"mutable"|"volatile"|"register"|"restrict"|"constexpr"|"consteval"|"constinit"|"friend"|"alignas"|"alignof"|"_Alignas"|"_Alignof"|"_Noreturn"|"_Thread_local"|"thread_local"|"decltype"|"typeof"|"typeof_unqual"|"concept"|"requires"|"export"|"noexcept"|"operator"|"FILE"|"std::string"|"std::vector"|"std::map"|"std::set"|"std::pair"|"std::tuple"|"std::array"|"std::deque"|"std::list"|"std::stack"|"std::queue" {
    return TYPE_KEYWORD;
}

"..."                   { return ELLIPSIS; }
"::"                    { return SCOPE; }
"->"                    { return ARROW; }
"."                     { return DOT; }
"?"                     { return QUESTION; }
":"                     { return COLON; }
"&"                     { return AMP; }
"|"                     { return PIPE; }
"^"                     { return CARET; }
"~"                     { return TILDE; }

"++"                    { return INC_OP; }
"--"                    { return DEC_OP; }
"&&"                    { return AND_OP; }
"||"                    { return OR_OP; }
"!"                     { return NOT_OP; }
"<<"                    { return LSHIFT; }
">>"                    { return RSHIFT; }

"+="                    { return ADD_ASSIGN; }
"-="                    { return SUB_ASSIGN; }
"*="                    { return MUL_ASSIGN; }
"/="                    { return DIV_ASSIGN; }
"%="                    { return MOD_ASSIGN; }
"&="                    { return AND_ASSIGN; }
"|="                    { return OR_ASSIGN; }
"^="                    { return XOR_ASSIGN; }
"<<="                   { return SHL_ASSIGN; }
">>="                   { return SHR_ASSIGN; }

"=="                    { return EQ; }
"!="                    { return NE; }
"<="                    { return LE; }
">="                    { return GE; }
"<"                     { return LT; }
">"                     { return GT; }

"+"                     { return PLUS; }
"-"                     { return MINUS; }
"*"                     { return MUL; }
"/"                     { return DIV; }
"%"                     { return MOD; }
"="                     { return ASSIGN; }
"("                     { return LPAREN; }
")"                     { return RPAREN; }
"{"                     { return LBRACE; }
"}"                     { return RBRACE; }
"["                     { return LBRACKET; }
"]"                     { return RBRACKET; }
";"                     { return SEMI; }
","                     { return COMMA; }

\"([^\"\\]|\\.)*\"      {
    yylval.str_val = new std::string(yytext);
    return STRING_LITERAL;
}

'(\\.|[^\\'\n])+'       {
    if (yytext[1] == '\\') {
        switch (yytext[2]) {
            case 'n': yylval.int_val = '\n'; break;
            case 't': yylval.int_val = '\t'; break;
            case 'r': yylval.int_val = '\r'; break;
            case '0': yylval.int_val = '\0'; break;
            case '\\': yylval.int_val = '\\'; break;
            case '\'': yylval.int_val = '\''; break;
            default: yylval.int_val = yytext[2]; break;
        }
    } else {
        yylval.int_val = (unsigned char)yytext[1];
    }
    return NUMBER;
}

0[xX][0-9a-fA-F]+[uUlL]* {
    yylval.int_val = (int)std::strtoll(yytext, nullptr, 16);
    return NUMBER;
}

0[bB][01]+[uUlL]*       {
    yylval.int_val = (int)std::strtoll(yytext + 2, nullptr, 2);
    return NUMBER;
}

[0-9]+[uUlL]+           {
    yylval.int_val = (int)std::strtoll(yytext, nullptr, 0);
    return NUMBER;
}

([0-9]+\.[0-9]*|\.[0-9]+)([eE][+-]?[0-9]+)?[fFlL]?|[0-9]+[eE][+-]?[0-9]+[fFlL]? {
    yylval.int_val = (int)std::strtod(yytext, nullptr);
    return NUMBER;
}

[a-zA-Z_][a-zA-Z0-9_]*  {
    yylval.str_val = new std::string(yytext);
    return IDENTIFIER;
}

[0-9]+                  {
    yylval.int_val = std::atoi(yytext);
    return NUMBER;
}

.                       {
    std::cerr << "Lexical error: Unrecognized character '" << yytext 
              << "' on line " << yylineno << std::endl;
}

%%
