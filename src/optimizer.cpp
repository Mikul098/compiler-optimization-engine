/**
 * ==============================================================================
 * File: src/optimizer.cpp
 * Purpose: Implementation of optimizer passes and utility functions.
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

static bool hasControlFlowInstructions(const std::vector<TACInstruction>& instructions) {
    for (const auto& instr : instructions) {
        if (instr.op == OpCode::LABEL ||
            instr.op == OpCode::GOTO ||
            instr.op == OpCode::IF_ZERO ||
            instr.op == OpCode::IF_NOT_ZERO) {
            return true;
        }
    }
    return false;
}

void ConstantFoldingPass::optimize(std::vector<TACInstruction>& instructions) {
    std::unordered_map<std::string, int> knownConstants;

    for (auto& instr : instructions) {
        // Labels reset constant tracking to prevent incorrect propagation across branches
        if (instr.op == OpCode::LABEL) {
            knownConstants.clear();
            continue;
        }

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
                             instr.op == OpCode::DIV ||
                             instr.op == OpCode::MOD);

        bool isLogical = (instr.op == OpCode::AND ||
                          instr.op == OpCode::OR);

        bool isBitwise = (instr.op == OpCode::BAND ||
                          instr.op == OpCode::BOR ||
                          instr.op == OpCode::BXOR ||
                          instr.op == OpCode::SHL ||
                          instr.op == OpCode::SHR);

        bool isComparison = (instr.op == OpCode::EQ ||
                             instr.op == OpCode::NE ||
                             instr.op == OpCode::LT ||
                             instr.op == OpCode::GT ||
                             instr.op == OpCode::LE ||
                             instr.op == OpCode::GE);

        if (instr.op == OpCode::BNOT && isNumber(instr.arg1)) {
            int val1 = std::stoi(instr.arg1);
            instr.op = OpCode::ASSIGN;
            instr.arg1 = std::to_string(~val1);
            instr.arg2 = "";
        }

        if ((isArithmetic || isLogical || isBitwise || isComparison) && isNumber(instr.arg1) && isNumber(instr.arg2)) {
            int val1 = std::stoi(instr.arg1);
            int val2 = std::stoi(instr.arg2);
            int computedResult = 0;
            bool divisionByZero = false;

            switch (instr.op) {
                case OpCode::ADD: computedResult = val1 + val2; break;
                case OpCode::SUB: computedResult = val1 - val2; break;
                case OpCode::MUL: computedResult = val1 * val2; break;
                case OpCode::DIV:
                    if (val2 == 0) {
                        divisionByZero = true;
                    } else {
                        computedResult = val1 / val2;
                    }
                    break;
                case OpCode::MOD:
                    if (val2 == 0) {
                        divisionByZero = true;
                    } else {
                        computedResult = val1 % val2;
                    }
                    break;
                case OpCode::AND: computedResult = (val1 != 0 && val2 != 0) ? 1 : 0; break;
                case OpCode::OR:  computedResult = (val1 != 0 || val2 != 0) ? 1 : 0; break;
                case OpCode::BAND: computedResult = val1 & val2; break;
                case OpCode::BOR:  computedResult = val1 | val2; break;
                case OpCode::BXOR: computedResult = val1 ^ val2; break;
                case OpCode::SHL:  computedResult = val1 << val2; break;
                case OpCode::SHR:  computedResult = val1 >> val2; break;
                case OpCode::EQ: computedResult = (val1 == val2) ? 1 : 0; break;
                case OpCode::NE: computedResult = (val1 != val2) ? 1 : 0; break;
                case OpCode::LT: computedResult = (val1 < val2) ? 1 : 0; break;
                case OpCode::GT: computedResult = (val1 > val2) ? 1 : 0; break;
                case OpCode::LE: computedResult = (val1 <= val2) ? 1 : 0; break;
                case OpCode::GE: computedResult = (val1 >= val2) ? 1 : 0; break;
                default: break;
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
            knownConstants.erase(instr.result);
        }
    }
}

void DeadCodeEliminationPass::optimize(std::vector<TACInstruction>& instructions) {
    if (hasControlFlowInstructions(instructions)) {
        // Safe global dead code elimination across control flow
        std::unordered_set<std::string> allReadVars;
        for (const auto& instr : instructions) {
            if (!instr.arg1.empty() && !isNumber(instr.arg1)) allReadVars.insert(instr.arg1);
            if (!instr.arg2.empty() && !isNumber(instr.arg2)) allReadVars.insert(instr.arg2);
        }

        for (auto& instr : instructions) {
            bool isValueProducer = (instr.op == OpCode::ASSIGN || instr.op == OpCode::ADD ||
                                    instr.op == OpCode::SUB || instr.op == OpCode::MUL ||
                                    instr.op == OpCode::DIV || instr.op == OpCode::MOD ||
                                    instr.op == OpCode::AND || instr.op == OpCode::OR ||
                                    instr.op == OpCode::BAND || instr.op == OpCode::BOR ||
                                    instr.op == OpCode::BXOR || instr.op == OpCode::BNOT ||
                                    instr.op == OpCode::SHL || instr.op == OpCode::SHR ||
                                    instr.op == OpCode::EQ || instr.op == OpCode::NE ||
                                    instr.op == OpCode::LT || instr.op == OpCode::GT ||
                                    instr.op == OpCode::LE || instr.op == OpCode::GE);

            if (isValueProducer && !instr.result.empty()) {
                if (allReadVars.find(instr.result) == allReadVars.end()) {
                    instr.op = OpCode::NOP;
                    instr.arg1 = "";
                    instr.arg2 = "";
                    instr.result = "";
                }
            }
        }

        instructions.erase(std::remove_if(instructions.begin(), instructions.end(), [](const TACInstruction& instr) { return instr.op == OpCode::NOP; }), instructions.end());
        return;
    }

    // Straight-line backward liveness analysis
    std::unordered_set<std::string> liveVariables;

    bool hasPrint = false;
    for (const auto& instr : instructions) {
        if (instr.op == OpCode::PRINT) {
            hasPrint = true;
            break;
        }
    }

    if (!hasPrint) {
        for (const auto& instr : instructions) {
            if (!instr.result.empty() && !(instr.result.length() >= 2 && instr.result[0] == 't' && std::isdigit(static_cast<unsigned char>(instr.result[1])))) {
                liveVariables.insert(instr.result);
            }
        }
    }

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
                   instr.op == OpCode::DIV ||
                   instr.op == OpCode::MOD ||
                   instr.op == OpCode::AND ||
                   instr.op == OpCode::OR ||
                   instr.op == OpCode::BAND ||
                   instr.op == OpCode::BOR ||
                   instr.op == OpCode::BXOR ||
                   instr.op == OpCode::BNOT ||
                   instr.op == OpCode::SHL ||
                   instr.op == OpCode::SHR ||
                   instr.op == OpCode::EQ ||
                   instr.op == OpCode::NE ||
                   instr.op == OpCode::LT ||
                   instr.op == OpCode::GT ||
                   instr.op == OpCode::LE ||
                   instr.op == OpCode::GE) {
            if (liveVariables.find(instr.result) != liveVariables.end()) {
                liveVariables.erase(instr.result);

                if (!instr.arg1.empty() && !isNumber(instr.arg1)) {
                    liveVariables.insert(instr.arg1);
                }
                if (!instr.arg2.empty() && !isNumber(instr.arg2)) {
                    liveVariables.insert(instr.arg2);
                }
            } else {
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
        case OpCode::MOD:    opStr = "MOD"; break;
        case OpCode::AND:    opStr = "AND"; break;
        case OpCode::OR:     opStr = "OR"; break;
        case OpCode::BAND:   opStr = "BAND"; break;
        case OpCode::BOR:    opStr = "BOR"; break;
        case OpCode::BXOR:   opStr = "BXOR"; break;
        case OpCode::BNOT:   opStr = "BNOT"; break;
        case OpCode::SHL:    opStr = "SHL"; break;
        case OpCode::SHR:    opStr = "SHR"; break;
        case OpCode::EQ:     opStr = "EQ"; break;
        case OpCode::NE:     opStr = "NE"; break;
        case OpCode::LT:     opStr = "LT"; break;
        case OpCode::GT:     opStr = "GT"; break;
        case OpCode::LE:     opStr = "LE"; break;
        case OpCode::GE:     opStr = "GE"; break;
        case OpCode::ASSIGN: opStr = "ASSIGN"; break;
        case OpCode::PRINT:  opStr = "PRINT"; break;
        case OpCode::NOP:    opStr = "NOP"; break;
        default:             opStr = "UNKNOWN"; break;
    }

    std::string a1 = arg1;
    std::string a2 = arg2;

    // Commutative operators: ADD, MUL, AND, OR, BAND, BOR, BXOR, EQ, NE
    bool isCommutative = (op == OpCode::ADD || op == OpCode::MUL || op == OpCode::AND || op == OpCode::OR ||
                          op == OpCode::BAND || op == OpCode::BOR || op == OpCode::BXOR ||
                          op == OpCode::EQ || op == OpCode::NE);
    if (isCommutative && a1 > a2) {
        std::swap(a1, a2);
    }

    return opStr + " " + a1 + " " + a2;
}

struct AvailableExprEntry {
    std::string resultVar;
    std::string arg1;
    std::string arg2;
};

void CommonSubexpressionPass::optimize(std::vector<TACInstruction>& instructions) {
    std::unordered_map<std::string, AvailableExprEntry> availableExpressions;

    for (auto& instruction : instructions) {
        if (instruction.op == OpCode::LABEL) {
            availableExpressions.clear();
            continue;
        }

        bool isMathOp = (instruction.op == OpCode::ADD || 
                         instruction.op == OpCode::SUB || 
                         instruction.op == OpCode::MUL || 
                         instruction.op == OpCode::DIV ||
                         instruction.op == OpCode::EQ ||
                         instruction.op == OpCode::NE ||
                         instruction.op == OpCode::LT ||
                         instruction.op == OpCode::GT ||
                         instruction.op == OpCode::LE ||
                         instruction.op == OpCode::GE);

        if (isMathOp) {
            std::string signature = getExpressionSignature(instruction.op, instruction.arg1, instruction.arg2);

            auto it = availableExpressions.find(signature);
            if (it != availableExpressions.end()) {
                instruction.op = OpCode::ASSIGN;
                instruction.arg1 = it->second.resultVar;
                instruction.arg2 = "";
            } else {
                availableExpressions[signature] = { instruction.result, instruction.arg1, instruction.arg2 };
            }
        }

        if (!instruction.result.empty()) {
            for (auto it = availableExpressions.begin(); it != availableExpressions.end(); ) {
                if (it->second.arg1 == instruction.result ||
                    it->second.arg2 == instruction.result ||
                    it->second.resultVar == instruction.result) {
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
    int iterations = 0;
    const int MAX_PASS_ITERATIONS = 50;

    while (changed && iterations++ < MAX_PASS_ITERATIONS) {
        changed = false;

        std::string beforeState;
        for (const auto& instr : instructions) {
            beforeState += instr.toString() + "\n";
        }

        for (auto& pass : passes) {
            if (pass) {
                pass->optimize(instructions);
            }
        }

        std::string afterState;
        for (const auto& instr : instructions) {
            afterState += instr.toString() + "\n";
        }

        if (beforeState != afterState) {
            changed = true;
        }
    }
}
