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


