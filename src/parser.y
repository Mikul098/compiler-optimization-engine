%{
#include "ast.h"
#include <iostream>
#include <string>
#include <memory>
#include <vector>

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
    BlockNode* block_node;
    std::vector<std::unique_ptr<ExpressionNode>>* expr_list;
}

%token PRINT PRINTF COUT ENDL LSHIFT RSHIFT CIN
%token PLUS MINUS MUL DIV MOD ASSIGN
%token ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN
%token AND_ASSIGN OR_ASSIGN XOR_ASSIGN SHL_ASSIGN SHR_ASSIGN
%token INC_OP DEC_OP AND_OP OR_OP NOT_OP
%token AMP PIPE CARET TILDE QUESTION COLON DOT ARROW SCOPE ELLIPSIS
%token LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET SEMI COMMA
%token TYPE_KEYWORD RETURN MAIN
%token IF ELSE WHILE FOR DO SWITCH CASE DEFAULT BREAK CONTINUE GOTO SIZEOF TRY CATCH THROW
%token STATIC_CAST DYNAMIC_CAST CONST_CAST REINTERPRET_CAST STATIC_ASSERT NEW DELETE
%token CO_AWAIT CO_RETURN CO_YIELD
%token EQ NE LT GT LE GE
%token COUT_PREC
%token <str_val> STRING_LITERAL
%token <str_val> IDENTIFIER
%token <int_val> NUMBER

%type <expr_node> expr opt_expr
%type <stmt_node> stmt block opt_for_init opt_for_inc var_decl_item cout_item cout_stmt cin_stmt
%type <block_node> block_stmt_list var_decl_list printf_args cout_chain
%type <str_val> qualified_id
%type <expr_list> arg_list opt_arg_list

%nonassoc IFX
%nonassoc ELSE

%right QUESTION COLON
%left OR_OP
%left AND_OP
%left PIPE
%left CARET
%left AMP
%left EQ NE
%left LT GT LE GE
%left LSHIFT RSHIFT
%left PLUS MINUS
%left MUL DIV MOD
%right NOT_OP TILDE UMINUS DEREF ADDR INC_OP DEC_OP
%left DOT ARROW SCOPE
%right COUT_PREC

%%

program:
    init items
    ;

init:
    /* empty */ {
        if (!root) {
            root = std::make_unique<ProgramNode>();
        }
    }
    ;

items:
    /* empty */
    | items item
    ;

item:
    stmt {
        if ($1 != nullptr && root) {
            root->addStatement(std::unique_ptr<StatementNode>($1));
        }
    }
    | func_def
    | STATIC_ASSERT LPAREN expr RPAREN SEMI {
        delete $3;
    }
    | STATIC_ASSERT LPAREN expr COMMA STRING_LITERAL RPAREN SEMI {
        delete $3;
        delete $5;
    }
    | type_specifier IDENTIFIER LBRACE block_stmt_list RBRACE SEMI {
        delete $2;
        delete $4;
    }
    | type_specifier LBRACE block_stmt_list RBRACE SEMI {
        delete $3;
    }
    | type_specifier IDENTIFIER SEMI {
        delete $2;
    }
    | type_specifier SEMI
    | SEMI
    ;

type_specifier:
    TYPE_KEYWORD
    | type_specifier TYPE_KEYWORD
    | type_specifier MUL
    | type_specifier AMP
    | type_specifier LT type_specifier GT
    | type_specifier LT type_specifier COMMA type_specifier GT
    ;

func_def:
    type_specifier func_name LPAREN opt_params RPAREN LBRACE block_stmt_list RBRACE {
        if (root && $7 != nullptr) {
            root->addStatement(std::unique_ptr<StatementNode>($7));
        }
    }
    | func_name LPAREN opt_params RPAREN LBRACE block_stmt_list RBRACE {
        if (root && $6 != nullptr) {
            root->addStatement(std::unique_ptr<StatementNode>($6));
        }
    }
    ;

func_name:
    MAIN
    | qualified_id { delete $1; }
    ;

opt_params:
    /* empty */
    | param_list
    ;

param_list:
    param
    | param_list COMMA param
    ;

param:
    param_tokens
    | ELLIPSIS
    ;

param_tokens:
    param_token
    | param_tokens param_token
    ;

param_token:
    TYPE_KEYWORD
    | IDENTIFIER { delete $1; }
    | MUL
    | AMP
    | LBRACKET
    | RBRACKET
    | NUMBER
    | LT
    | GT
    ;

block:
    LBRACE block_stmt_list RBRACE {
        $$ = $2;
    }
    ;

block_stmt_list:
    /* empty */ {
        $$ = new BlockNode();
    }
    | block_stmt_list stmt {
        if ($2 != nullptr) {
            $1->addStatement(std::unique_ptr<StatementNode>($2));
        }
        $$ = $1;
    }
    | block_stmt_list SEMI {
        $$ = $1;
    }
    ;

var_decl_list:
    var_decl_item {
        $$ = new BlockNode();
        if ($1 != nullptr) {
            $$->addStatement(std::unique_ptr<StatementNode>($1));
        }
    }
    | var_decl_list COMMA var_decl_item {
        if ($3 != nullptr) {
            $1->addStatement(std::unique_ptr<StatementNode>($3));
        }
        $$ = $1;
    }
    ;

var_decl_item:
    IDENTIFIER {
        $$ = new AssignmentNode(*$1, std::make_unique<NumberNode>(0));
        delete $1;
    }
    | IDENTIFIER ASSIGN expr {
        $$ = new AssignmentNode(*$1, std::unique_ptr<ExpressionNode>($3));
        delete $1;
    }
    | IDENTIFIER LBRACKET NUMBER RBRACKET {
        $$ = new AssignmentNode(*$1 + "[0]", std::make_unique<NumberNode>(0));
        delete $1;
    }
    | IDENTIFIER LBRACKET RBRACKET {
        $$ = new AssignmentNode(*$1 + "[0]", std::make_unique<NumberNode>(0));
        delete $1;
    }
    | IDENTIFIER LBRACKET NUMBER RBRACKET ASSIGN LBRACE opt_arg_list RBRACE {
        auto block = new BlockNode();
        if ($7 != nullptr) {
            for (size_t i = 0; i < $7->size(); ++i) {
                block->addStatement(std::make_unique<AssignmentNode>(*$1 + "[" + std::to_string(i) + "]", std::move((*$7)[i])));
            }
            delete $7;
        }
        delete $1;
        $$ = block;
    }
    | IDENTIFIER LBRACKET RBRACKET ASSIGN LBRACE opt_arg_list RBRACE {
        auto block = new BlockNode();
        if ($6 != nullptr) {
            for (size_t i = 0; i < $6->size(); ++i) {
                block->addStatement(std::make_unique<AssignmentNode>(*$1 + "[" + std::to_string(i) + "]", std::move((*$6)[i])));
            }
            delete $6;
        }
        delete $1;
        $$ = block;
    }
    ;

qualified_id:
    IDENTIFIER {
        $$ = $1;
    }
    | SCOPE IDENTIFIER {
        $$ = new std::string("::" + *$2);
        delete $2;
    }
    | IDENTIFIER SCOPE IDENTIFIER {
        $$ = new std::string(*$1 + "::" + *$3);
        delete $1;
        delete $3;
    }
    ;

stmt:
    qualified_id ASSIGN expr SEMI {
        $$ = new AssignmentNode(*$1, std::unique_ptr<ExpressionNode>($3));
        delete $1;
    }
    | type_specifier var_decl_list SEMI {
        $$ = $2;
    }
    | qualified_id DOT IDENTIFIER ASSIGN expr SEMI {
        $$ = new AssignmentNode(*$1 + "." + *$3, std::unique_ptr<ExpressionNode>($5));
        delete $1; delete $3;
    }
    | qualified_id ARROW IDENTIFIER ASSIGN expr SEMI {
        $$ = new AssignmentNode(*$1 + "->" + *$3, std::unique_ptr<ExpressionNode>($5));
        delete $1; delete $3;
    }
    | qualified_id LBRACKET expr RBRACKET ASSIGN expr SEMI {
        $$ = new AssignmentNode(*$1 + "[]", std::unique_ptr<ExpressionNode>($6));
        delete $1; delete $3;
    }
    | MUL qualified_id ASSIGN expr SEMI {
        $$ = new AssignmentNode("*" + *$2, std::unique_ptr<ExpressionNode>($4));
        delete $2;
    }
    | qualified_id ADD_ASSIGN expr SEMI {
        $$ = new AssignmentNode(*$1, std::make_unique<BinaryOpNode>(BinaryOp::ADD, std::make_unique<VariableNode>(*$1), std::unique_ptr<ExpressionNode>($3)));
        delete $1;
    }
    | qualified_id SUB_ASSIGN expr SEMI {
        $$ = new AssignmentNode(*$1, std::make_unique<BinaryOpNode>(BinaryOp::SUB, std::make_unique<VariableNode>(*$1), std::unique_ptr<ExpressionNode>($3)));
        delete $1;
    }
    | qualified_id MUL_ASSIGN expr SEMI {
        $$ = new AssignmentNode(*$1, std::make_unique<BinaryOpNode>(BinaryOp::MUL, std::make_unique<VariableNode>(*$1), std::unique_ptr<ExpressionNode>($3)));
        delete $1;
    }
    | qualified_id DIV_ASSIGN expr SEMI {
        $$ = new AssignmentNode(*$1, std::make_unique<BinaryOpNode>(BinaryOp::DIV, std::make_unique<VariableNode>(*$1), std::unique_ptr<ExpressionNode>($3)));
        delete $1;
    }
    | qualified_id MOD_ASSIGN expr SEMI {
        $$ = new AssignmentNode(*$1, std::make_unique<BinaryOpNode>(BinaryOp::MOD, std::make_unique<VariableNode>(*$1), std::unique_ptr<ExpressionNode>($3)));
        delete $1;
    }
    | qualified_id AND_ASSIGN expr SEMI {
        $$ = new AssignmentNode(*$1, std::make_unique<BinaryOpNode>(BinaryOp::BAND, std::make_unique<VariableNode>(*$1), std::unique_ptr<ExpressionNode>($3)));
        delete $1;
    }
    | qualified_id OR_ASSIGN expr SEMI {
        $$ = new AssignmentNode(*$1, std::make_unique<BinaryOpNode>(BinaryOp::BOR, std::make_unique<VariableNode>(*$1), std::unique_ptr<ExpressionNode>($3)));
        delete $1;
    }
    | qualified_id XOR_ASSIGN expr SEMI {
        $$ = new AssignmentNode(*$1, std::make_unique<BinaryOpNode>(BinaryOp::BXOR, std::make_unique<VariableNode>(*$1), std::unique_ptr<ExpressionNode>($3)));
        delete $1;
    }
    | qualified_id SHL_ASSIGN expr SEMI {
        $$ = new AssignmentNode(*$1, std::make_unique<BinaryOpNode>(BinaryOp::SHL, std::make_unique<VariableNode>(*$1), std::unique_ptr<ExpressionNode>($3)));
        delete $1;
    }
    | qualified_id SHR_ASSIGN expr SEMI {
        $$ = new AssignmentNode(*$1, std::make_unique<BinaryOpNode>(BinaryOp::SHR, std::make_unique<VariableNode>(*$1), std::unique_ptr<ExpressionNode>($3)));
        delete $1;
    }
    | qualified_id INC_OP SEMI {
        $$ = new AssignmentNode(*$1, std::make_unique<BinaryOpNode>(BinaryOp::ADD, std::make_unique<VariableNode>(*$1), std::make_unique<NumberNode>(1)));
        delete $1;
    }
    | INC_OP qualified_id SEMI {
        $$ = new AssignmentNode(*$2, std::make_unique<BinaryOpNode>(BinaryOp::ADD, std::make_unique<VariableNode>(*$2), std::make_unique<NumberNode>(1)));
        delete $2;
    }
    | qualified_id DEC_OP SEMI {
        $$ = new AssignmentNode(*$1, std::make_unique<BinaryOpNode>(BinaryOp::SUB, std::make_unique<VariableNode>(*$1), std::make_unique<NumberNode>(1)));
        delete $1;
    }
    | DEC_OP qualified_id SEMI {
        $$ = new AssignmentNode(*$2, std::make_unique<BinaryOpNode>(BinaryOp::SUB, std::make_unique<VariableNode>(*$2), std::make_unique<NumberNode>(1)));
        delete $2;
    }
    | IF LPAREN expr RPAREN stmt %prec IFX {
        $$ = new IfNode(std::unique_ptr<ExpressionNode>($3), std::unique_ptr<StatementNode>($5), nullptr);
    }
    | IF LPAREN expr RPAREN stmt ELSE stmt {
        $$ = new IfNode(std::unique_ptr<ExpressionNode>($3), std::unique_ptr<StatementNode>($5), std::unique_ptr<StatementNode>($7));
    }
    | WHILE LPAREN expr RPAREN stmt {
        $$ = new WhileNode(std::unique_ptr<ExpressionNode>($3), std::unique_ptr<StatementNode>($5));
    }
    | DO stmt WHILE LPAREN expr RPAREN SEMI {
        $$ = new DoWhileNode(std::unique_ptr<StatementNode>($2), std::unique_ptr<ExpressionNode>($5));
    }
    | FOR LPAREN opt_for_init SEMI opt_expr SEMI opt_for_inc RPAREN stmt {
        $$ = new ForNode(std::unique_ptr<StatementNode>($3), std::unique_ptr<ExpressionNode>($5), std::unique_ptr<StatementNode>($7), std::unique_ptr<StatementNode>($9));
    }
    | FOR LPAREN type_specifier IDENTIFIER COLON expr RPAREN stmt {
        delete $4;
        delete $6;
        $$ = $8;
    }
    | SWITCH LPAREN expr RPAREN stmt {
        $$ = new SwitchNode(std::unique_ptr<ExpressionNode>($3), std::unique_ptr<StatementNode>($5));
    }
    | CASE NUMBER COLON {
        $$ = new CaseNode($2);
    }
    | CASE MINUS NUMBER COLON {
        $$ = new CaseNode(-$3);
    }
    | DEFAULT COLON {
        $$ = new DefaultNode();
    }
    | BREAK SEMI {
        $$ = new BreakNode();
    }
    | CONTINUE SEMI {
        $$ = new ContinueNode();
    }
    | GOTO qualified_id SEMI {
        $$ = new GotoNode(*$2);
        delete $2;
    }
    | IDENTIFIER COLON {
        $$ = new LabelNode(*$1);
        delete $1;
    }
    | qualified_id LPAREN opt_arg_list RPAREN SEMI {
        delete $1;
        delete $3;
        $$ = nullptr;
    }
    | cout_stmt {
        $$ = $1;
    }
    | cin_stmt {
        $$ = $1;
    }
    | TRY block catch_list {
        $$ = $2;
    }
    | THROW expr SEMI {
        delete $2;
        $$ = nullptr;
    }
    | THROW SEMI {
        $$ = nullptr;
    }
    | DELETE expr SEMI {
        delete $2;
        $$ = nullptr;
    }
    | DELETE LBRACKET RBRACKET expr SEMI {
        delete $4;
        $$ = nullptr;
    }
    | CO_RETURN expr SEMI {
        delete $2;
        $$ = nullptr;
    }
    | CO_RETURN SEMI {
        $$ = nullptr;
    }
    | CO_YIELD expr SEMI {
        delete $2;
        $$ = nullptr;
    }
    | STATIC_ASSERT LPAREN expr RPAREN SEMI {
        delete $3;
        $$ = nullptr;
    }
    | STATIC_ASSERT LPAREN expr COMMA STRING_LITERAL RPAREN SEMI {
        delete $3;
        delete $5;
        $$ = nullptr;
    }
    | block {
        $$ = $1;
    }
    | PRINT expr SEMI {
        $$ = new PrintNode(std::unique_ptr<ExpressionNode>($2));
    }
    | PRINT LPAREN expr RPAREN SEMI {
        $$ = new PrintNode(std::unique_ptr<ExpressionNode>($3));
    }
    | PRINTF LPAREN STRING_LITERAL COMMA printf_args RPAREN SEMI {
        delete $3;
        $$ = $5;
    }
    | PRINTF LPAREN STRING_LITERAL RPAREN SEMI {
        delete $3;
        $$ = nullptr;
    }
    | PRINTF LPAREN expr RPAREN SEMI {
        $$ = new PrintNode(std::unique_ptr<ExpressionNode>($3));
    }
    | RETURN expr SEMI {
        $$ = nullptr;
    }
    | RETURN SEMI {
        $$ = nullptr;
    }
    ;

cout_stmt:
    COUT cout_chain SEMI {
        $$ = $2;
    }
    ;

cout_chain:
    LSHIFT cout_item {
        $$ = new BlockNode();
        if ($2 != nullptr) {
            $$->addStatement(std::unique_ptr<StatementNode>($2));
        }
    }
    | cout_chain LSHIFT cout_item {
        if ($3 != nullptr) {
            $1->addStatement(std::unique_ptr<StatementNode>($3));
        }
        $$ = $1;
    }
    ;

cout_item:
    expr %prec COUT_PREC {
        $$ = new PrintNode(std::unique_ptr<ExpressionNode>($1));
    }
    | STRING_LITERAL {
        delete $1;
        $$ = nullptr;
    }
    | ENDL {
        $$ = nullptr;
    }
    ;

cin_stmt:
    CIN cin_chain SEMI {
        $$ = nullptr;
    }
    ;

cin_chain:
    RSHIFT qualified_id {
        delete $2;
    }
    | cin_chain RSHIFT qualified_id {
        delete $3;
    }
    ;

catch_list:
    catch_clause
    | catch_list catch_clause
    ;

catch_clause:
    CATCH LPAREN opt_params RPAREN block {
        delete $5;
    }
    ;

opt_for_init:
    /* empty */ { $$ = nullptr; }
    | qualified_id ASSIGN expr {
        $$ = new AssignmentNode(*$1, std::unique_ptr<ExpressionNode>($3));
        delete $1;
    }
    | type_specifier var_decl_list {
        $$ = $2;
    }
    ;

opt_expr:
    /* empty */ { $$ = nullptr; }
    | expr { $$ = $1; }
    ;

opt_for_inc:
    /* empty */ { $$ = nullptr; }
    | qualified_id ASSIGN expr {
        $$ = new AssignmentNode(*$1, std::unique_ptr<ExpressionNode>($3));
        delete $1;
    }
    | qualified_id ADD_ASSIGN expr {
        $$ = new AssignmentNode(*$1, std::make_unique<BinaryOpNode>(BinaryOp::ADD, std::make_unique<VariableNode>(*$1), std::unique_ptr<ExpressionNode>($3)));
        delete $1;
    }
    | qualified_id SUB_ASSIGN expr {
        $$ = new AssignmentNode(*$1, std::make_unique<BinaryOpNode>(BinaryOp::SUB, std::make_unique<VariableNode>(*$1), std::unique_ptr<ExpressionNode>($3)));
        delete $1;
    }
    | qualified_id MUL_ASSIGN expr {
        $$ = new AssignmentNode(*$1, std::make_unique<BinaryOpNode>(BinaryOp::MUL, std::make_unique<VariableNode>(*$1), std::unique_ptr<ExpressionNode>($3)));
        delete $1;
    }
    | qualified_id DIV_ASSIGN expr {
        $$ = new AssignmentNode(*$1, std::make_unique<BinaryOpNode>(BinaryOp::DIV, std::make_unique<VariableNode>(*$1), std::unique_ptr<ExpressionNode>($3)));
        delete $1;
    }
    | qualified_id MOD_ASSIGN expr {
        $$ = new AssignmentNode(*$1, std::make_unique<BinaryOpNode>(BinaryOp::MOD, std::make_unique<VariableNode>(*$1), std::unique_ptr<ExpressionNode>($3)));
        delete $1;
    }
    | qualified_id AND_ASSIGN expr {
        $$ = new AssignmentNode(*$1, std::make_unique<BinaryOpNode>(BinaryOp::BAND, std::make_unique<VariableNode>(*$1), std::unique_ptr<ExpressionNode>($3)));
        delete $1;
    }
    | qualified_id OR_ASSIGN expr {
        $$ = new AssignmentNode(*$1, std::make_unique<BinaryOpNode>(BinaryOp::BOR, std::make_unique<VariableNode>(*$1), std::unique_ptr<ExpressionNode>($3)));
        delete $1;
    }
    | qualified_id XOR_ASSIGN expr {
        $$ = new AssignmentNode(*$1, std::make_unique<BinaryOpNode>(BinaryOp::BXOR, std::make_unique<VariableNode>(*$1), std::unique_ptr<ExpressionNode>($3)));
        delete $1;
    }
    | qualified_id SHL_ASSIGN expr {
        $$ = new AssignmentNode(*$1, std::make_unique<BinaryOpNode>(BinaryOp::SHL, std::make_unique<VariableNode>(*$1), std::unique_ptr<ExpressionNode>($3)));
        delete $1;
    }
    | qualified_id SHR_ASSIGN expr {
        $$ = new AssignmentNode(*$1, std::make_unique<BinaryOpNode>(BinaryOp::SHR, std::make_unique<VariableNode>(*$1), std::unique_ptr<ExpressionNode>($3)));
        delete $1;
    }
    | qualified_id INC_OP {
        $$ = new AssignmentNode(*$1, std::make_unique<BinaryOpNode>(BinaryOp::ADD, std::make_unique<VariableNode>(*$1), std::make_unique<NumberNode>(1)));
        delete $1;
    }
    | INC_OP qualified_id {
        $$ = new AssignmentNode(*$2, std::make_unique<BinaryOpNode>(BinaryOp::ADD, std::make_unique<VariableNode>(*$2), std::make_unique<NumberNode>(1)));
        delete $2;
    }
    | qualified_id DEC_OP {
        $$ = new AssignmentNode(*$1, std::make_unique<BinaryOpNode>(BinaryOp::SUB, std::make_unique<VariableNode>(*$1), std::make_unique<NumberNode>(1)));
        delete $1;
    }
    | DEC_OP qualified_id {
        $$ = new AssignmentNode(*$2, std::make_unique<BinaryOpNode>(BinaryOp::SUB, std::make_unique<VariableNode>(*$2), std::make_unique<NumberNode>(1)));
        delete $2;
    }
    ;

printf_args:
    expr {
        $$ = new BlockNode();
        $$->addStatement(std::make_unique<PrintNode>(std::unique_ptr<ExpressionNode>($1)));
    }
    | printf_args COMMA expr {
        $1->addStatement(std::make_unique<PrintNode>(std::unique_ptr<ExpressionNode>($3)));
        $$ = $1;
    }
    ;

opt_arg_list:
    /* empty */ { $$ = new std::vector<std::unique_ptr<ExpressionNode>>(); }
    | arg_list { $$ = $1; }
    ;

arg_list:
    expr {
        $$ = new std::vector<std::unique_ptr<ExpressionNode>>();
        $$->push_back(std::unique_ptr<ExpressionNode>($1));
    }
    | arg_list COMMA expr {
        $1->push_back(std::unique_ptr<ExpressionNode>($3));
        $$ = $1;
    }
    ;

expr:
    expr QUESTION expr COLON expr {
        $$ = new TernaryNode(std::unique_ptr<ExpressionNode>($1), std::unique_ptr<ExpressionNode>($3), std::unique_ptr<ExpressionNode>($5));
    }
    | expr PLUS expr {
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
    | expr MOD expr {
        $$ = new BinaryOpNode(BinaryOp::MOD, std::unique_ptr<ExpressionNode>($1), std::unique_ptr<ExpressionNode>($3));
    }
    | expr AND_OP expr {
        $$ = new BinaryOpNode(BinaryOp::AND, std::unique_ptr<ExpressionNode>($1), std::unique_ptr<ExpressionNode>($3));
    }
    | expr OR_OP expr {
        $$ = new BinaryOpNode(BinaryOp::OR, std::unique_ptr<ExpressionNode>($1), std::unique_ptr<ExpressionNode>($3));
    }
    | expr AMP expr {
        $$ = new BinaryOpNode(BinaryOp::BAND, std::unique_ptr<ExpressionNode>($1), std::unique_ptr<ExpressionNode>($3));
    }
    | expr PIPE expr {
        $$ = new BinaryOpNode(BinaryOp::BOR, std::unique_ptr<ExpressionNode>($1), std::unique_ptr<ExpressionNode>($3));
    }
    | expr CARET expr {
        $$ = new BinaryOpNode(BinaryOp::BXOR, std::unique_ptr<ExpressionNode>($1), std::unique_ptr<ExpressionNode>($3));
    }
    | expr LSHIFT expr {
        $$ = new BinaryOpNode(BinaryOp::SHL, std::unique_ptr<ExpressionNode>($1), std::unique_ptr<ExpressionNode>($3));
    }
    | expr RSHIFT expr {
        $$ = new BinaryOpNode(BinaryOp::SHR, std::unique_ptr<ExpressionNode>($1), std::unique_ptr<ExpressionNode>($3));
    }
    | expr EQ expr {
        $$ = new BinaryOpNode(BinaryOp::EQ, std::unique_ptr<ExpressionNode>($1), std::unique_ptr<ExpressionNode>($3));
    }
    | expr NE expr {
        $$ = new BinaryOpNode(BinaryOp::NE, std::unique_ptr<ExpressionNode>($1), std::unique_ptr<ExpressionNode>($3));
    }
    | expr LT expr {
        $$ = new BinaryOpNode(BinaryOp::LT, std::unique_ptr<ExpressionNode>($1), std::unique_ptr<ExpressionNode>($3));
    }
    | expr GT expr {
        $$ = new BinaryOpNode(BinaryOp::GT, std::unique_ptr<ExpressionNode>($1), std::unique_ptr<ExpressionNode>($3));
    }
    | expr LE expr {
        $$ = new BinaryOpNode(BinaryOp::LE, std::unique_ptr<ExpressionNode>($1), std::unique_ptr<ExpressionNode>($3));
    }
    | expr GE expr {
        $$ = new BinaryOpNode(BinaryOp::GE, std::unique_ptr<ExpressionNode>($1), std::unique_ptr<ExpressionNode>($3));
    }
    | NOT_OP expr {
        $$ = new BinaryOpNode(BinaryOp::EQ, std::unique_ptr<ExpressionNode>($2), std::make_unique<NumberNode>(0));
    }
    | TILDE expr %prec TILDE {
        $$ = new BinaryOpNode(BinaryOp::BXOR, std::unique_ptr<ExpressionNode>($2), std::make_unique<NumberNode>(-1));
    }
    | MINUS expr %prec UMINUS {
        $$ = new BinaryOpNode(BinaryOp::SUB, std::make_unique<NumberNode>(0), std::unique_ptr<ExpressionNode>($2));
    }
    | PLUS expr %prec UMINUS {
        $$ = $2;
    }
    | AMP qualified_id %prec ADDR {
        $$ = new VariableNode("&" + *$2);
        delete $2;
    }
    | MUL qualified_id %prec DEREF {
        $$ = new VariableNode("*" + *$2);
        delete $2;
    }
    | SIZEOF LPAREN expr RPAREN {
        delete $3;
        $$ = new NumberNode(4);
    }
    | SIZEOF LPAREN type_specifier RPAREN {
        $$ = new NumberNode(4);
    }
    | LPAREN type_specifier RPAREN expr %prec UMINUS {
        $$ = $4;
    }
    | STATIC_CAST LT type_specifier GT LPAREN expr RPAREN {
        $$ = $6;
    }
    | DYNAMIC_CAST LT type_specifier GT LPAREN expr RPAREN {
        $$ = $6;
    }
    | CONST_CAST LT type_specifier GT LPAREN expr RPAREN {
        $$ = $6;
    }
    | REINTERPRET_CAST LT type_specifier GT LPAREN expr RPAREN {
        $$ = $6;
    }
    | NEW type_specifier {
        $$ = new NumberNode(1);
    }
    | NEW type_specifier LBRACKET expr RBRACKET {
        delete $4;
        $$ = new NumberNode(1);
    }
    | CO_AWAIT expr {
        $$ = $2;
    }
    | qualified_id LPAREN opt_arg_list RPAREN {
        $$ = new FunctionCallNode(*$1, $3 != nullptr ? std::move(*$3) : std::vector<std::unique_ptr<ExpressionNode>>());
        delete $1;
        delete $3;
    }
    | qualified_id INC_OP {
        $$ = new VariableNode(*$1);
        delete $1;
    }
    | INC_OP qualified_id {
        $$ = new VariableNode(*$2);
        delete $2;
    }
    | qualified_id DEC_OP {
        $$ = new VariableNode(*$1);
        delete $1;
    }
    | DEC_OP qualified_id {
        $$ = new VariableNode(*$2);
        delete $2;
    }
    | qualified_id DOT IDENTIFIER {
        $$ = new VariableNode(*$1 + "." + *$3);
        delete $1; delete $3;
    }
    | qualified_id ARROW IDENTIFIER {
        $$ = new VariableNode(*$1 + "->" + *$3);
        delete $1; delete $3;
    }
    | qualified_id LBRACKET expr RBRACKET {
        $$ = new VariableNode(*$1 + "[]");
        delete $1; delete $3;
    }
    | LPAREN expr RPAREN {
        $$ = $2;
    }
    | qualified_id {
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
