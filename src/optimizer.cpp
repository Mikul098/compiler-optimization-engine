/**
 * ==============================================================================
 * File: src/optimizer.cpp
 * Purpose: Implementation of optimizer passes and utility functions.
 *
 * Role in Compiler:
 *  - Implements helper functions such as isNumber.
 *  - Implements ConstantFoldingPass for TAC instruction streams.
 * ==============================================================================
 */

#include "optimizer.h"
#include <cctype>
#include <algorithm>
#include <iostream>

/**
 * Safely checks if a string represents a valid integer (handling optional negative sign).
 */
bool isNumber(const std::string& str) {
    if (str.empty()) {
        return false;
    }
    size_t start = 0;
    if (str[0] == '-') {
        if (str.length() == 1) {
            return false;
        }
        start = 1;
    }
    for (size_t i = start; i < str.length(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(str[i]))) {
            return false;
        }
    }
    return true;
}

void ConstantFoldingPass::optimize(std::vector<TACInstruction>& instructions) {
    std::unordered_map<std::string, int> knownConstants;

    for (auto& instr : instructions) {
        // a. Propagation
        auto it1 = knownConstants.find(instr.arg1);
        if (it1 != knownConstants.end()) {
            instr.arg1 = std::to_string(it1->second);
        }

        auto it2 = knownConstants.find(instr.arg2);
        if (it2 != knownConstants.end()) {
            instr.arg2 = std::to_string(it2->second);
        }

        // b. Folding
        bool isArithmetic = (instr.op == OpCode::ADD || 
                             instr.op == OpCode::SUB || 
                             instr.op == OpCode::MUL || 
                             instr.op == OpCode::DIV);

        if (isArithmetic && isNumber(instr.arg1) && isNumber(instr.arg2)) {
            int val1 = std::stoi(instr.arg1);
            int val2 = std::stoi(instr.arg2);
            int computedResult = 0;
            bool divisionByZero = false;

            switch (instr.op) {
                case OpCode::ADD:
                    computedResult = val1 + val2;
                    break;
                case OpCode::SUB:
                    computedResult = val1 - val2;
                    break;
                case OpCode::MUL:
                    computedResult = val1 * val2;
                    break;
                case OpCode::DIV:
                    if (val2 == 0) {
                        divisionByZero = true;
                    } else {
                        computedResult = val1 / val2;
                    }
                    break;
                default:
                    break;
            }

            if (!divisionByZero) {
                instr.op = OpCode::ASSIGN;
                instr.arg1 = std::to_string(computedResult);
                instr.arg2 = "";
            }
        }

        // c. Tracking & Invalidation
        if (instr.op == OpCode::ASSIGN) {
            if (isNumber(instr.arg1)) {
                knownConstants[instr.result] = std::stoi(instr.arg1);
            } else {
                knownConstants.erase(instr.result);
            }
        } else if (!instr.result.empty()) {
            // Any other instruction that defines/writes to result invalidates previous constant value
            knownConstants.erase(instr.result);
        }
    }
}

void DeadCodeEliminationPass::optimize(std::vector<TACInstruction>& instructions) {
    std::unordered_set<std::string> liveVariables;

    for (int i = static_cast<int>(instructions.size()) - 1; i >= 0; --i) {
        auto& instr = instructions[i];

        if (instr.op == OpCode::PRINT) {
            if (!instr.arg1.empty() && !isNumber(instr.arg1)) {
                liveVariables.insert(instr.arg1);
            }
        } else if (instr.op == OpCode::ASSIGN ||
                   instr.op == OpCode::ADD ||
                   instr.op == OpCode::SUB ||
                   instr.op == OpCode::MUL ||
                   instr.op == OpCode::DIV) {
            if (liveVariables.find(instr.result) != liveVariables.end()) {
                // Variable is needed later
                liveVariables.erase(instr.result);

                if (!instr.arg1.empty() && !isNumber(instr.arg1)) {
                    liveVariables.insert(instr.arg1);
                }
                if (!instr.arg2.empty() && !isNumber(instr.arg2)) {
                    liveVariables.insert(instr.arg2);
                }
            } else {
                // Variable is never used again (dead code)
                instr.op = OpCode::NOP;
                instr.arg1 = "";
                instr.arg2 = "";
                instr.result = "";
            }
        }
    }

    instructions.erase(std::remove_if(instructions.begin(), instructions.end(), [](const TACInstruction& instr) { return instr.op == OpCode::NOP; }), instructions.end());
}

std::string CommonSubexpressionPass::getExpressionSignature(OpCode op, const std::string& arg1, const std::string& arg2) {
    std::string opStr;
    switch (op) {
        case OpCode::ADD:    opStr = "ADD"; break;
        case OpCode::SUB:    opStr = "SUB"; break;
        case OpCode::MUL:    opStr = "MUL"; break;
        case OpCode::DIV:    opStr = "DIV"; break;
        case OpCode::ASSIGN: opStr = "ASSIGN"; break;
        case OpCode::PRINT:  opStr = "PRINT"; break;
        case OpCode::NOP:    opStr = "NOP"; break;
        default:             opStr = "UNKNOWN"; break;
    }

    std::string a1 = arg1;
    std::string a2 = arg2;

    // Bonus for Commutativity: ADD and MUL are commutative (x + y == y + x, x * y == y * x).
    // Sort operands alphabetically so that x * y and y * x produce the exact same signature.
    if ((op == OpCode::ADD || op == OpCode::MUL) && a1 > a2) {
        std::swap(a1, a2);
    }

    return opStr + " " + a1 + " " + a2;
}

void CommonSubexpressionPass::optimize(std::vector<TACInstruction>& instructions) {
    // 1. Initialize map from expression signature to the variable holding its result
    std::unordered_map<std::string, std::string> availableExpressions;

    // 2. Iterate FORWARD through the instructions vector by reference
    for (auto& instruction : instructions) {
        // a. Substitution:
        // If the instruction is a math operation (ADD, SUB, MUL, DIV)
        bool isMathOp = (instruction.op == OpCode::ADD || 
                         instruction.op == OpCode::SUB || 
                         instruction.op == OpCode::MUL || 
                         instruction.op == OpCode::DIV);

        if (isMathOp) {
            std::string signature = getExpressionSignature(instruction.op, instruction.arg1, instruction.arg2);

            auto it = availableExpressions.find(signature);
            if (it != availableExpressions.end()) {
                // The math is redundant! Change this instruction's op to ASSIGN
                instruction.op = OpCode::ASSIGN;
                instruction.arg1 = it->second;
                instruction.arg2 = "";
            } else {
                // If the signature does NOT exist: add it to the dictionary
                availableExpressions[signature] = instruction.result;
            }
        }

        // b. Invalidation:
        // If the instruction assigns a new value to a variable (any operation that overwrites result)
        if (!instruction.result.empty()) {
            for (auto it = availableExpressions.begin(); it != availableExpressions.end(); ) {
                if (it->first.find(instruction.result) != std::string::npos) {
                    it = availableExpressions.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }
}

void PassManager::addPass(std::unique_ptr<OptimizerPass> pass) {
    passes.push_back(std::move(pass));
}

void PassManager::optimize(std::vector<TACInstruction>& instructions) {
    bool changed = true;

    while (changed) {
        changed = false;

        // b. Generate string representation of the current instructions state before passes
        std::string beforeState;
        for (const auto& instr : instructions) {
            beforeState += instr.toString() + "\n";
        }

        // c. Iterate through the passes vector and call pass->optimize(instructions)
        for (auto& pass : passes) {
            if (pass) {
                pass->optimize(instructions);
            }
        }

        // d. Generate new string representation of the instructions state after passes
        std::string afterState;
        for (const auto& instr : instructions) {
            afterState += instr.toString() + "\n";
        }

        // e. If state changed, set changed = true to force another iteration
        if (beforeState != afterState) {
            changed = true;
            // f. Optional logging
            std::cout << "Pass Manager: Iteration complete. Changes detected.\n";
        }
    }
}

