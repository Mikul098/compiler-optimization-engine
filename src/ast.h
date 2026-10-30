/**
 * ==============================================================================
 * File: src/ast.h
 * Purpose: Header file defining the Abstract Syntax Tree (AST) node hierarchy.
 *
 * Role in Compiler:
 *  - Core intermediate representation representing the parsed program structure.
 *  - Declares the abstract base class ASTNode and derived classes:
 *      * StatementNode   : Abstract base for statements.
 *      * ExpressionNode  : Abstract base for value-evaluating expressions.
 *      * ProgramNode     : Root node holding a sequence of statements.
 *      * BlockNode       : Scoped block of statements { ... }.
 *      * IfNode          : Conditional branching (if ... else ...).
 *      * WhileNode       : Loop construct (while ...).
 *      * ForNode         : Loop construct (for ...).
 *      * AssignmentNode  : Variable assignment statement (e.g., x = expr;).
 *      * PrintNode       : Console print statement (e.g., print(expr);).
 *      * BinaryOpNode    : Arithmetic and relational operations (+, -, *, /, ==, !=, <, >, <=, >=).
 *      * VariableNode    : Identifier/variable reference expression.
 *      * NumberNode      : Integer literal numeric expression.
 * ==============================================================================
 */

#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include "tac.h"

// Abstract base class for all AST nodes
class ASTNode {
public:
    virtual ~ASTNode() = default;
    
    // Pretty-print the node and its children in terminal using box-drawing characters
    virtual void print(const std::string& prefix = "", bool isLast = true, const std::string& label = "") const = 0;

    // Render node as HTML for visual browser tree
    virtual void toHtml(std::ostream& out) const = 0;

    // Export node hierarchy to JSON format for image rendering
    virtual void toJson(std::ostream& out, const std::string& edge = "") const = 0;

    // Generate Three-Address Code
    virtual std::string generateTAC(TACBuilder& builder) const = 0;
};

// Base class for statements
class StatementNode : public ASTNode {
public:
    virtual ~StatementNode() = default;
};

// Base class for expressions
class ExpressionNode : public ASTNode {
public:
    virtual ~ExpressionNode() = default;
};

// Root node representing a sequence of statements
class ProgramNode : public ASTNode {
public:
    std::vector<std::unique_ptr<StatementNode>> statements;

    void addStatement(std::unique_ptr<StatementNode> stmt) {
        statements.push_back(std::move(stmt));
    }

    void print(const std::string& prefix = "", bool isLast = true, const std::string& label = "") const override;
    void toHtml(std::ostream& out) const override;
    void toJson(std::ostream& out, const std::string& edge = "") const override;
    std::string generateTAC(TACBuilder& builder) const override;
    void exportHTML(const std::string& filename) const;
    void exportJSON(const std::string& filename) const;
    void exportPNG(const std::string& filename) const;
};

// Scoped block of statements: { ... }
class BlockNode : public StatementNode {
public:
    std::vector<std::unique_ptr<StatementNode>> statements;

    void addStatement(std::unique_ptr<StatementNode> stmt) {
        statements.push_back(std::move(stmt));
    }

    void print(const std::string& prefix = "", bool isLast = true, const std::string& label = "") const override;
    void toHtml(std::ostream& out) const override;
    void toJson(std::ostream& out, const std::string& edge = "") const override;
    std::string generateTAC(TACBuilder& builder) const override;
};

// Conditional branching: if (cond) thenBranch else elseBranch
class IfNode : public StatementNode {
public:
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<StatementNode> thenBranch;
    std::unique_ptr<StatementNode> elseBranch;

    IfNode(std::unique_ptr<ExpressionNode> cond, std::unique_ptr<StatementNode> thenB, std::unique_ptr<StatementNode> elseB = nullptr)
        : condition(std::move(cond)), thenBranch(std::move(thenB)), elseBranch(std::move(elseB)) {}

    void print(const std::string& prefix = "", bool isLast = true, const std::string& label = "") const override;
    void toHtml(std::ostream& out) const override;
    void toJson(std::ostream& out, const std::string& edge = "") const override;
    std::string generateTAC(TACBuilder& builder) const override;
};

// While loop: while (cond) body
class WhileNode : public StatementNode {
public:
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<StatementNode> body;

    WhileNode(std::unique_ptr<ExpressionNode> cond, std::unique_ptr<StatementNode> body)
        : condition(std::move(cond)), body(std::move(body)) {}

    void print(const std::string& prefix = "", bool isLast = true, const std::string& label = "") const override;
    void toHtml(std::ostream& out) const override;
    void toJson(std::ostream& out, const std::string& edge = "") const override;
    std::string generateTAC(TACBuilder& builder) const override;
};

// For loop: for (init; cond; inc) body
class ForNode : public StatementNode {
public:
    std::unique_ptr<StatementNode> init;
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<StatementNode> increment;
    std::unique_ptr<StatementNode> body;

    ForNode(std::unique_ptr<StatementNode> init, std::unique_ptr<ExpressionNode> cond, std::unique_ptr<StatementNode> inc, std::unique_ptr<StatementNode> body)
        : init(std::move(init)), condition(std::move(cond)), increment(std::move(inc)), body(std::move(body)) {}

    void print(const std::string& prefix = "", bool isLast = true, const std::string& label = "") const override;
    void toHtml(std::ostream& out) const override;
    void toJson(std::ostream& out, const std::string& edge = "") const override;
    std::string generateTAC(TACBuilder& builder) const override;
};

// Variable assignment: e.g., x = 10;
class AssignmentNode : public StatementNode {
public:
    std::string varName;
    std::unique_ptr<ExpressionNode> expression;

    AssignmentNode(std::string name, std::unique_ptr<ExpressionNode> expr)
        : varName(std::move(name)), expression(std::move(expr)) {}

    void print(const std::string& prefix = "", bool isLast = true, const std::string& label = "") const override;
    void toHtml(std::ostream& out) const override;
    void toJson(std::ostream& out, const std::string& edge = "") const override;
    std::string generateTAC(TACBuilder& builder) const override;
};

// Print statement: e.g., print(x);
class PrintNode : public StatementNode {
public:
    std::unique_ptr<ExpressionNode> expression;

    explicit PrintNode(std::unique_ptr<ExpressionNode> expr)
        : expression(std::move(expr)) {}

    void print(const std::string& prefix = "", bool isLast = true, const std::string& label = "") const override;
    void toHtml(std::ostream& out) const override;
    void toJson(std::ostream& out, const std::string& edge = "") const override;
    std::string generateTAC(TACBuilder& builder) const override;
};

// Supported binary operators
enum class BinaryOp {
    ADD, SUB, MUL, DIV, MOD, AND, OR,
    BAND, BOR, BXOR, SHL, SHR,
    EQ, NE, LT, GT, LE, GE
};

// Do-While loop: do body while (cond);
class DoWhileNode : public StatementNode {
public:
    std::unique_ptr<StatementNode> body;
    std::unique_ptr<ExpressionNode> condition;

    DoWhileNode(std::unique_ptr<StatementNode> body, std::unique_ptr<ExpressionNode> cond)
        : body(std::move(body)), condition(std::move(cond)) {}

    void print(const std::string& prefix = "", bool isLast = true, const std::string& label = "") const override;
    void toHtml(std::ostream& out) const override;
    void toJson(std::ostream& out, const std::string& edge = "") const override;
    std::string generateTAC(TACBuilder& builder) const override;
};

// Break statement: break;
class BreakNode : public StatementNode {
public:
    void print(const std::string& prefix = "", bool isLast = true, const std::string& label = "") const override;
    void toHtml(std::ostream& out) const override;
    void toJson(std::ostream& out, const std::string& edge = "") const override;
    std::string generateTAC(TACBuilder& builder) const override;
};

// Continue statement: continue;
class ContinueNode : public StatementNode {
public:
    void print(const std::string& prefix = "", bool isLast = true, const std::string& label = "") const override;
    void toHtml(std::ostream& out) const override;
    void toJson(std::ostream& out, const std::string& edge = "") const override;
    std::string generateTAC(TACBuilder& builder) const override;
};

// Case label: case <value>:
class CaseNode : public StatementNode {
public:
    int value;
    mutable std::string label;

    explicit CaseNode(int val) : value(val) {}

    void print(const std::string& prefix = "", bool isLast = true, const std::string& label = "") const override;
    void toHtml(std::ostream& out) const override;
    void toJson(std::ostream& out, const std::string& edge = "") const override;
    std::string generateTAC(TACBuilder& builder) const override;
};

// Default label: default:
class DefaultNode : public StatementNode {
public:
    mutable std::string label;

    DefaultNode() = default;

    void print(const std::string& prefix = "", bool isLast = true, const std::string& label = "") const override;
    void toHtml(std::ostream& out) const override;
    void toJson(std::ostream& out, const std::string& edge = "") const override;
    std::string generateTAC(TACBuilder& builder) const override;
};

// Switch statement: switch (expr) body
class SwitchNode : public StatementNode {
public:
    std::unique_ptr<ExpressionNode> expression;
    std::unique_ptr<StatementNode> body;

    SwitchNode(std::unique_ptr<ExpressionNode> expr, std::unique_ptr<StatementNode> body)
        : expression(std::move(expr)), body(std::move(body)) {}

    void print(const std::string& prefix = "", bool isLast = true, const std::string& label = "") const override;
    void toHtml(std::ostream& out) const override;
    void toJson(std::ostream& out, const std::string& edge = "") const override;
    std::string generateTAC(TACBuilder& builder) const override;
};

// Goto statement: goto label;
class GotoNode : public StatementNode {
public:
    std::string labelName;

    explicit GotoNode(std::string name) : labelName(std::move(name)) {}

    void print(const std::string& prefix = "", bool isLast = true, const std::string& label = "") const override;
    void toHtml(std::ostream& out) const override;
    void toJson(std::ostream& out, const std::string& edge = "") const override;
    std::string generateTAC(TACBuilder& builder) const override;
};

// Label declaration: label:
class LabelNode : public StatementNode {
public:
    std::string labelName;

    explicit LabelNode(std::string name) : labelName(std::move(name)) {}

    void print(const std::string& prefix = "", bool isLast = true, const std::string& label = "") const override;
    void toHtml(std::ostream& out) const override;
    void toJson(std::ostream& out, const std::string& edge = "") const override;
    std::string generateTAC(TACBuilder& builder) const override;
};

// Binary arithmetic / comparison operation
class BinaryOpNode : public ExpressionNode {
public:
    BinaryOp op;
    std::unique_ptr<ExpressionNode> left;
    std::unique_ptr<ExpressionNode> right;

    BinaryOpNode(BinaryOp op, std::unique_ptr<ExpressionNode> left, std::unique_ptr<ExpressionNode> right)
        : op(op), left(std::move(left)), right(std::move(right)) {}

    void print(const std::string& prefix = "", bool isLast = true, const std::string& label = "") const override;
    void toHtml(std::ostream& out) const override;
    void toJson(std::ostream& out, const std::string& edge = "") const override;
    std::string generateTAC(TACBuilder& builder) const override;
};

// Variable identifier leaf
class VariableNode : public ExpressionNode {
public:
    std::string name;

    explicit VariableNode(std::string name) : name(std::move(name)) {}

    void print(const std::string& prefix = "", bool isLast = true, const std::string& label = "") const override;
    void toHtml(std::ostream& out) const override;
    void toJson(std::ostream& out, const std::string& edge = "") const override;
    std::string generateTAC(TACBuilder& builder) const override;
};

// Integer literal leaf
class NumberNode : public ExpressionNode {
public:
    int value;

    explicit NumberNode(int value) : value(value) {}

    void print(const std::string& prefix = "", bool isLast = true, const std::string& label = "") const override;
    void toHtml(std::ostream& out) const override;
    void toJson(std::ostream& out, const std::string& edge = "") const override;
    std::string generateTAC(TACBuilder& builder) const override;
};

// Ternary operator: condition ? thenExpr : elseExpr
class TernaryNode : public ExpressionNode {
public:
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<ExpressionNode> thenExpr;
    std::unique_ptr<ExpressionNode> elseExpr;

    TernaryNode(std::unique_ptr<ExpressionNode> cond,
                std::unique_ptr<ExpressionNode> tExpr,
                std::unique_ptr<ExpressionNode> eExpr)
        : condition(std::move(cond)), thenExpr(std::move(tExpr)), elseExpr(std::move(eExpr)) {}

    void print(const std::string& prefix = "", bool isLast = true, const std::string& label = "") const override;
    void toHtml(std::ostream& out) const override;
    void toJson(std::ostream& out, const std::string& edge = "") const override;
    std::string generateTAC(TACBuilder& builder) const override;
};

// Function call: func(arg1, arg2, ...)
class FunctionCallNode : public ExpressionNode {
public:
    std::string funcName;
    std::vector<std::unique_ptr<ExpressionNode>> args;

    FunctionCallNode(std::string name, std::vector<std::unique_ptr<ExpressionNode>> a)
        : funcName(std::move(name)), args(std::move(a)) {}

    void print(const std::string& prefix = "", bool isLast = true, const std::string& label = "") const override;
    void toHtml(std::ostream& out) const override;
    void toJson(std::ostream& out, const std::string& edge = "") const override;
    std::string generateTAC(TACBuilder& builder) const override;
};
