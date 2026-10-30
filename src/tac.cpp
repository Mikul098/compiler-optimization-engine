/**
 * ==============================================================================
 * File: src/tac.cpp
 * Purpose: Implementation of Three-Address Code (TAC) emission and formatting.
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
        case OpCode::MOD:
            return result + " = " + arg1 + " % " + arg2;
        case OpCode::AND:
            return result + " = " + arg1 + " && " + arg2;
        case OpCode::OR:
            return result + " = " + arg1 + " || " + arg2;
        case OpCode::BAND:
            return result + " = " + arg1 + " & " + arg2;
        case OpCode::BOR:
            return result + " = " + arg1 + " | " + arg2;
        case OpCode::BXOR:
            return result + " = " + arg1 + " ^ " + arg2;
        case OpCode::BNOT:
            return result + " = ~" + arg1;
        case OpCode::SHL:
            return result + " = " + arg1 + " << " + arg2;
        case OpCode::SHR:
            return result + " = " + arg1 + " >> " + arg2;
        case OpCode::EQ:
            return result + " = " + arg1 + " == " + arg2;
        case OpCode::NE:
            return result + " = " + arg1 + " != " + arg2;
        case OpCode::LT:
            return result + " = " + arg1 + " < " + arg2;
        case OpCode::GT:
            return result + " = " + arg1 + " > " + arg2;
        case OpCode::LE:
            return result + " = " + arg1 + " <= " + arg2;
        case OpCode::GE:
            return result + " = " + arg1 + " >= " + arg2;
        case OpCode::LABEL:
            return arg1 + ":";
        case OpCode::GOTO:
            return "goto " + arg1;
        case OpCode::IF_ZERO:
            return "if_false " + arg1 + " goto " + arg2;
        case OpCode::IF_NOT_ZERO:
            return "if_true " + arg1 + " goto " + arg2;
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

std::string TACBuilder::newLabel() {
    return "L" + std::to_string(labelCounter++);
}

void TACBuilder::emit(OpCode op, std::string arg1, std::string arg2, std::string result) {
    instructions.push_back({op, arg1, arg2, result});
}

void TACBuilder::printAll() const {
    for (const auto& instr : instructions) {
        std::cout << instr.toString() << "\n";
    }
}
