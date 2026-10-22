/**
 * ==============================================================================
 * File: src/tac.h
 * Purpose: Header file defining Three-Address Code (TAC) intermediate structures.
 *
 * Role in Compiler:
 *  - Intermediate Representation (IR) layer bridging AST and future target code.
 *  - Defines:
 *      * OpCode enum class    : Core operations, comparisons, and control flow jumps.
 *      * TACInstruction struct : Represents a flattened quad instruction.
 *      * TACBuilder class     : Manages temporary variable allocation (t1, t2, ...)
 *                               and label allocation (L1, L2, ...).
 * ==============================================================================
 */

#pragma once
#include <string>
#include <vector>
#include <iostream>

enum class OpCode {
    ADD, SUB, MUL, DIV, MOD, AND, OR,
    BAND, BOR, BXOR, BNOT, SHL, SHR,
    ASSIGN, PRINT, NOP,
    // Relational Operators
    EQ, NE, LT, GT, LE, GE,
    // Control Flow Instructions
    LABEL, GOTO, IF_ZERO, IF_NOT_ZERO
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
    int labelCounter;
    std::vector<std::string> continueLabels;
    std::vector<std::string> breakLabels;

public:
    TACBuilder() : tempCounter(1), labelCounter(1) {}

    std::string newTemp();
    std::string newLabel();
    void emit(OpCode op, std::string arg1, std::string arg2, std::string result);
    void printAll() const;
    const std::vector<TACInstruction>& getInstructions() const { return instructions; }
    std::vector<TACInstruction>& getInstructions() { return instructions; }

    void pushContinueLabel(const std::string& l) { continueLabels.push_back(l); }
    void popContinueLabel() { if (!continueLabels.empty()) continueLabels.pop_back(); }
    std::string currentContinueLabel() const { return continueLabels.empty() ? "" : continueLabels.back(); }

    void pushBreakLabel(const std::string& l) { breakLabels.push_back(l); }
    void popBreakLabel() { if (!breakLabels.empty()) breakLabels.pop_back(); }
    std::string currentBreakLabel() const { return breakLabels.empty() ? "" : breakLabels.back(); }
};
