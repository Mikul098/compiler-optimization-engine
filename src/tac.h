/**
 * ==============================================================================
 * File: src/tac.h
 * Purpose: Header file defining Three-Address Code (TAC) intermediate structures.
 *
 * Role in Compiler:
 *  - Intermediate Representation (IR) layer bridging AST and future target code.
 *  - Defines:
 *      * OpCode enum class    : Core operations (ADD, SUB, MUL, DIV, ASSIGN, PRINT, NOP).
 *      * TACInstruction struct : Represents a flattened quad instruction (result = arg1 op arg2).
 *      * TACBuilder class     : Manages temporary variable allocation (t1, t2, ...)
 *                               and linear instruction emission/printing.
 * ==============================================================================
 */

#pragma once
#include <string>
#include <vector>
#include <iostream>

enum class OpCode {
    ADD, SUB, MUL, DIV, ASSIGN, PRINT, NOP
};

struct TACInstruction {
    OpCode op;
    std::string arg1;
    std::string arg2;
    std::string result;

    std::string toString() const;
};

class TACBuilder {
private:
    std::vector<TACInstruction> instructions;
    int tempCounter;

public:
    TACBuilder() : tempCounter(1) {}

    std::string newTemp();
    void emit(OpCode op, std::string arg1, std::string arg2, std::string result);
    void printAll() const;
    const std::vector<TACInstruction>& getInstructions() const { return instructions; }
    std::vector<TACInstruction>& getInstructions() { return instructions; }
};
