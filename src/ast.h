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
 *      * AssignmentNode  : Variable assignment statement (e.g., x = expr;).
 *      * PrintNode       : Console print statement (e.g., print(expr);).
 *      * BinaryOpNode    : Binary arithmetic operations (+, -, *, /).
 *      * VariableNode    : Identifier/variable reference expression.
 *      * NumberNode      : Integer literal numeric expression.
 *  - Declares polymorphic interfaces for:
 *      * Terminal pretty-printing using ANSI colors and box-drawing glyphs.
 *      * HTML and JSON serialization for visualization.
 *      * Three-Address Code (TAC) intermediate generation via post-order traversal.
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
    ADD,
    SUB,
    MUL,
    DIV
};

// Binary arithmetic operation
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
