/*
 * ==============================================================================
 * File: src/parser.y
 * Purpose: Syntax Analyzer (Parser) grammar specification for GNU Bison.
 *
 * Role in Compiler:
 *  - Syntactic analysis phase: consumes tokens produced by the lexer (yylex)
 *    and verifies conformance to the Toy language context-free grammar.
 *  - Enforces operator precedence and associativity (%left PLUS MINUS, %left MUL DIV).
 *  - Constructs the Abstract Syntax Tree (AST) in memory using polymorphic AST nodes.
 *  - Stores the parsed program root into the global variable 'root' (std::unique_ptr<ProgramNode>).
 *  - Detects syntax errors and invokes yyerror() with the line number from yylineno.
 * ==============================================================================
 */

%{
#include "ast.h"
#include <iostream>
#include <string>
#include <memory>

int yylex();
void yyerror(const char* s);

extern int yylineno;

// Global AST root pointer
std::unique_ptr<ProgramNode> root;
%}

%union {
    int int_val;
    std::string* str_val;
    ExpressionNode* expr_node;
    StatementNode* stmt_node;
}

%token PRINT
%token PLUS MINUS MUL DIV ASSIGN
%token LPAREN RPAREN SEMI

%token <str_val> IDENTIFIER
%token <int_val> NUMBER

%type <expr_node> expr
%type <stmt_node> stmt

%left PLUS MINUS
%left MUL DIV

%%

program:
    /* empty */ {
        root = std::make_unique<ProgramNode>();
    }
    | program stmt {
        if ($2 != nullptr) {
            root->addStatement(std::unique_ptr<StatementNode>($2));
        }
    }
    ;

stmt:
    IDENTIFIER ASSIGN expr SEMI {
        $$ = new AssignmentNode(*$1, std::unique_ptr<ExpressionNode>($3));
        delete $1; // Free the string pointer allocated by flex
    }
    | PRINT LPAREN expr RPAREN SEMI {
        $$ = new PrintNode(std::unique_ptr<ExpressionNode>($3));
    }
    ;

expr:
    expr PLUS expr {
        $$ = new BinaryOpNode(BinaryOp::ADD, std::unique_ptr<ExpressionNode>($1), std::unique_ptr<ExpressionNode>($3));
    }
    | expr MINUS expr {
        $$ = new BinaryOpNode(BinaryOp::SUB, std::unique_ptr<ExpressionNode>($1), std::unique_ptr<ExpressionNode>($3));
    }
    | expr MUL expr {
        $$ = new BinaryOpNode(BinaryOp::MUL, std::unique_ptr<ExpressionNode>($1), std::unique_ptr<ExpressionNode>($3));
    }
    | expr DIV expr {
        $$ = new BinaryOpNode(BinaryOp::DIV, std::unique_ptr<ExpressionNode>($1), std::unique_ptr<ExpressionNode>($3));
    }
    | LPAREN expr RPAREN {
        $$ = $2;
    }
    | IDENTIFIER {
        $$ = new VariableNode(*$1);
        delete $1;
    }
    | NUMBER {
        $$ = new NumberNode($1);
    }
    ;

%%

void yyerror(const char* s) {
    std::cerr << "Syntax Error on line " << yylineno << ": " << s << std::endl;
}
