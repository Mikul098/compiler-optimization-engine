/**
 * ==============================================================================
 * File: src/tac.cpp
 * Purpose: Implementation of Three-Address Code (TAC) emission and formatting.
 *
 * Role in Compiler:
 *  - Implements TACInstruction::toString() to format instructions into
 *    human-readable three-address code strings (e.g., "t1 = a + b", "print x").
 *  - Implements TACBuilder:
 *      * newTemp()  : Allocates consecutive unique temporary variables ("t1", "t2", ...).
 *      * emit()     : Appends newly synthesized instructions to the instruction list.
 *      * printAll() : Dumps the full linear sequence of TAC instructions to stdout.
 * ==============================================================================
 */

#include "tac.h"

std::string TACInstruction::toString() const {
    switch (op) {
        case OpCode::ADD:
            return result + " = " + arg1 + " + " + arg2;
        case OpCode::SUB:
            return result + " = " + arg1 + " - " + arg2;
        case OpCode::MUL:
            return result + " = " + arg1 + " * " + arg2;
        case OpCode::DIV:
            return result + " = " + arg1 + " / " + arg2;
        case OpCode::ASSIGN:
            return result + " = " + arg1;
        case OpCode::PRINT:
            return "print " + arg1;
        case OpCode::NOP:
            return "nop";
        default:
            return "unknown";
    }
}

std::string TACBuilder::newTemp() {
    return "t" + std::to_string(tempCounter++);
}

void TACBuilder::emit(OpCode op, std::string arg1, std::string arg2, std::string result) {
    instructions.push_back({op, arg1, arg2, result});
}

void TACBuilder::printAll() const {
    for (const auto& instr : instructions) {
        std::cout << instr.toString() << "\n";
    }
}
