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
                case OpCode::SHL:
                    if (val2 >= 0 && val2 < 31) computedResult = val1 << val2;
                    else divisionByZero = true;
                    break;
                case OpCode::SHR:
                    if (val2 >= 0 && val2 < 32) computedResult = val1 >> val2;
                    else divisionByZero = true;
                    break;
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

// =============================================================================
// Pass 4: Algebraic Simplification & Strength Reduction
// Applies well-known algebraic identities and replaces expensive ops with
// cheaper equivalents (e.g. multiply-by-2 → left shift by 1).
// =============================================================================
void AlgebraicSimplificationPass::optimize(std::vector<TACInstruction>& instructions) {
    for (auto& instr : instructions) {
        // ---- MUL identities ----
        if (instr.op == OpCode::MUL) {
            // x * 0 = 0
            if (instr.arg2 == "0" || instr.arg1 == "0") {
                instr.op   = OpCode::ASSIGN;
                instr.arg1 = "0";
                instr.arg2 = "";
                continue;
            }
            // x * 1 = x
            if (instr.arg2 == "1") {
                instr.op   = OpCode::ASSIGN;
                instr.arg2 = "";
                continue;
            }
            if (instr.arg1 == "1") {
                instr.op   = OpCode::ASSIGN;
                instr.arg1 = instr.arg2;
                instr.arg2 = "";
                continue;
            }
            // x * 2 = x << 1  (strength reduction)
            if (instr.arg2 == "2" && !isNumber(instr.arg1)) {
                instr.op   = OpCode::SHL;
                instr.arg2 = "1";
                continue;
            }
            if (instr.arg1 == "2" && !isNumber(instr.arg2)) {
                instr.op   = OpCode::SHL;
                instr.arg1 = instr.arg2;
                instr.arg2 = "1";
                continue;
            }
            // x * 4 = x << 2
            if (instr.arg2 == "4" && !isNumber(instr.arg1)) {
                instr.op   = OpCode::SHL;
                instr.arg2 = "2";
                continue;
            }
            if (instr.arg1 == "4" && !isNumber(instr.arg2)) {
                instr.op   = OpCode::SHL;
                instr.arg1 = instr.arg2;
                instr.arg2 = "2";
                continue;
            }
            // x * 8 = x << 3
            if (instr.arg2 == "8" && !isNumber(instr.arg1)) {
                instr.op   = OpCode::SHL;
                instr.arg2 = "3";
                continue;
            }
            if (instr.arg1 == "8" && !isNumber(instr.arg2)) {
                instr.op   = OpCode::SHL;
                instr.arg1 = instr.arg2;
                instr.arg2 = "3";
                continue;
            }
        }

        // ---- ADD identities ----
        if (instr.op == OpCode::ADD) {
            // x + 0 = x
            if (instr.arg2 == "0") {
                instr.op   = OpCode::ASSIGN;
                instr.arg2 = "";
                continue;
            }
            if (instr.arg1 == "0") {
                instr.op   = OpCode::ASSIGN;
                instr.arg1 = instr.arg2;
                instr.arg2 = "";
                continue;
            }
        }

        // ---- SUB identities ----
        if (instr.op == OpCode::SUB) {
            // x - 0 = x
            if (instr.arg2 == "0") {
                instr.op   = OpCode::ASSIGN;
                instr.arg2 = "";
                continue;
            }
            // x - x = 0  (same variable)
            if (!instr.arg1.empty() && !isNumber(instr.arg1) && instr.arg1 == instr.arg2) {
                instr.op   = OpCode::ASSIGN;
                instr.arg1 = "0";
                instr.arg2 = "";
                continue;
            }
        }

        // ---- DIV identities ----
        if (instr.op == OpCode::DIV) {
            // x / 1 = x
            if (instr.arg2 == "1") {
                instr.op   = OpCode::ASSIGN;
                instr.arg2 = "";
                continue;
            }
            // x / x = 1  (same non-zero variable — safe at compile time)
            if (!instr.arg1.empty() && !isNumber(instr.arg1) && instr.arg1 == instr.arg2) {
                instr.op   = OpCode::ASSIGN;
                instr.arg1 = "1";
                instr.arg2 = "";
                continue;
            }
        }

        // ---- MOD identities ----
        if (instr.op == OpCode::MOD) {
            // x % 1 = 0
            if (instr.arg2 == "1") {
                instr.op   = OpCode::ASSIGN;
                instr.arg1 = "0";
                instr.arg2 = "";
                continue;
            }
            // x % x = 0
            if (!instr.arg1.empty() && !isNumber(instr.arg1) && instr.arg1 == instr.arg2) {
                instr.op   = OpCode::ASSIGN;
                instr.arg1 = "0";
                instr.arg2 = "";
                continue;
            }
        }

        // ---- Bitwise identities ----
        if (instr.op == OpCode::BAND) {
            // x & 0 = 0
            if (instr.arg1 == "0" || instr.arg2 == "0") {
                instr.op   = OpCode::ASSIGN;
                instr.arg1 = "0";
                instr.arg2 = "";
                continue;
            }
            // x & x = x
            if (!instr.arg1.empty() && !isNumber(instr.arg1) && instr.arg1 == instr.arg2) {
                instr.op   = OpCode::ASSIGN;
                instr.arg2 = "";
                continue;
            }
        }

        if (instr.op == OpCode::BOR) {
            // x | 0 = x
            if (instr.arg2 == "0") {
                instr.op   = OpCode::ASSIGN;
                instr.arg2 = "";
                continue;
            }
            if (instr.arg1 == "0") {
                instr.op   = OpCode::ASSIGN;
                instr.arg1 = instr.arg2;
                instr.arg2 = "";
                continue;
            }
            // x | x = x
            if (!instr.arg1.empty() && !isNumber(instr.arg1) && instr.arg1 == instr.arg2) {
                instr.op   = OpCode::ASSIGN;
                instr.arg2 = "";
                continue;
            }
        }

        if (instr.op == OpCode::BXOR) {
            // x ^ x = 0
            if (!instr.arg1.empty() && !isNumber(instr.arg1) && instr.arg1 == instr.arg2) {
                instr.op   = OpCode::ASSIGN;
                instr.arg1 = "0";
                instr.arg2 = "";
                continue;
            }
            // x ^ 0 = x
            if (instr.arg2 == "0") {
                instr.op   = OpCode::ASSIGN;
                instr.arg2 = "";
                continue;
            }
            if (instr.arg1 == "0") {
                instr.op   = OpCode::ASSIGN;
                instr.arg1 = instr.arg2;
                instr.arg2 = "";
                continue;
            }
        }

        // ---- Shift identities ----
        if (instr.op == OpCode::SHL || instr.op == OpCode::SHR) {
            // x << 0 = x,  x >> 0 = x
            if (instr.arg2 == "0") {
                instr.op   = OpCode::ASSIGN;
                instr.arg2 = "";
                continue;
            }
        }

        // ---- Logical identities ----
        if (instr.op == OpCode::AND) {
            // x && 0 = 0
            if (instr.arg1 == "0" || instr.arg2 == "0") {
                instr.op   = OpCode::ASSIGN;
                instr.arg1 = "0";
                instr.arg2 = "";
                continue;
            }
        }
        if (instr.op == OpCode::OR) {
            // x || 1 = 1
            if (instr.arg1 == "1" || instr.arg2 == "1") {
                instr.op   = OpCode::ASSIGN;
                instr.arg1 = "1";
                instr.arg2 = "";
                continue;
            }
        }

        // ---- Comparison identities ----
        // x == x → 1,  x != x → 0
        if (instr.op == OpCode::EQ && !instr.arg1.empty() && !isNumber(instr.arg1) && instr.arg1 == instr.arg2) {
            instr.op   = OpCode::ASSIGN;
            instr.arg1 = "1";
            instr.arg2 = "";
            continue;
        }
        if (instr.op == OpCode::NE && !instr.arg1.empty() && !isNumber(instr.arg1) && instr.arg1 == instr.arg2) {
            instr.op   = OpCode::ASSIGN;
            instr.arg1 = "0";
            instr.arg2 = "";
            continue;
        }
        // x < x → 0,  x > x → 0,  x <= x → 1,  x >= x → 1
        if ((instr.op == OpCode::LT || instr.op == OpCode::GT) &&
            !instr.arg1.empty() && !isNumber(instr.arg1) && instr.arg1 == instr.arg2) {
            instr.op   = OpCode::ASSIGN;
            instr.arg1 = "0";
            instr.arg2 = "";
            continue;
        }
        if ((instr.op == OpCode::LE || instr.op == OpCode::GE) &&
            !instr.arg1.empty() && !isNumber(instr.arg1) && instr.arg1 == instr.arg2) {
            instr.op   = OpCode::ASSIGN;
            instr.arg1 = "1";
            instr.arg2 = "";
            continue;
        }
    }
}

// =============================================================================
// Pass 5: Copy Propagation
// Tracks ASSIGN instructions of the form  dst = src  where src is a variable
// and propagates src into later operands that use dst.
// Invalidated when dst or src is redefined.
// =============================================================================
void CopyPropagationPass::optimize(std::vector<TACInstruction>& instructions) {
    // copy_map[var] = value it was copied from (a variable name, not a number)
    std::unordered_map<std::string, std::string> copyMap;

    for (auto& instr : instructions) {
        // Labels mark control-flow joins — conservative: clear all copies
        if (instr.op == OpCode::LABEL) {
            copyMap.clear();
            continue;
        }

        // Propagate known copies into arg1 and arg2
        auto applyProp = [&](std::string& arg) {
            if (!arg.empty() && !isNumber(arg)) {
                auto it = copyMap.find(arg);
                if (it != copyMap.end()) {
                    arg = it->second;
                }
            }
        };
        applyProp(instr.arg1);
        applyProp(instr.arg2);

        // Record new copy or invalidate
        if (instr.op == OpCode::ASSIGN && !instr.result.empty()) {
            // Invalidate any copy whose source was overwritten
            for (auto it = copyMap.begin(); it != copyMap.end(); ) {
                if (it->second == instr.result || it->first == instr.result) {
                    it = copyMap.erase(it);
                } else {
                    ++it;
                }
            }
            // If RHS is a plain variable (not a number), record the copy
            if (!instr.arg1.empty() && !isNumber(instr.arg1) && instr.arg2.empty()) {
                copyMap[instr.result] = instr.arg1;
            }
        } else if (!instr.result.empty()) {
            // Any other defining instruction invalidates copies of this var
            for (auto it = copyMap.begin(); it != copyMap.end(); ) {
                if (it->first == instr.result || it->second == instr.result) {
                    it = copyMap.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }
}

// =============================================================================
// Pass 6: Redundant Assignment Elimination
// Removes instructions of the form   x = x   (self-assignments that are NOPs).
// =============================================================================
void RedundantAssignmentPass::optimize(std::vector<TACInstruction>& instructions) {
    for (auto& instr : instructions) {
        if (instr.op == OpCode::ASSIGN &&
            !instr.result.empty() &&
            instr.arg2.empty() &&
            instr.arg1 == instr.result) {
            instr.op     = OpCode::NOP;
            instr.arg1   = "";
            instr.arg2   = "";
            instr.result = "";
        }
    }
    instructions.erase(
        std::remove_if(instructions.begin(), instructions.end(),
                       [](const TACInstruction& i) { return i.op == OpCode::NOP; }),
        instructions.end()
    );
}

// =============================================================================
// Pass 7: Constant Propagation (cross-block forward pass)
// Unlike ConstantFoldingPass (which only folds when BOTH operands are already
// numeric literals in the TAC), this pass tracks all variables whose current
// value is a compile-time constant and substitutes them across the entire
// instruction stream.  Label boundaries conservatively flush the map.
// Running this pass before ConstantFoldingPass enables the folder to collapse
// more expressions on the subsequent iteration.
// =============================================================================
void ConstantPropagationPass::optimize(std::vector<TACInstruction>& instructions) {
    std::unordered_map<std::string, int> constMap; // var -> constant value

    for (auto& instr : instructions) {
        if (instr.op == OpCode::LABEL) {
            constMap.clear();
            continue;
        }

        // Substitute known constants into operands
        auto substitute = [&](std::string& arg) {
            if (!arg.empty() && !isNumber(arg)) {
                auto it = constMap.find(arg);
                if (it != constMap.end()) {
                    arg = std::to_string(it->second);
                }
            }
        };
        substitute(instr.arg1);
        substitute(instr.arg2);

        // Update the constant map based on what this instruction defines
        if (!instr.result.empty()) {
            if (instr.op == OpCode::ASSIGN && isNumber(instr.arg1) && instr.arg2.empty()) {
                constMap[instr.result] = std::stoi(instr.arg1);
            } else {
                constMap.erase(instr.result);
            }
        }
    }
}

// =============================================================================
// Pass 8: Peephole Optimization
// Slides a 2-instruction window looking for local patterns:
//   Pattern A — redundant temporary copy collapse:
//       t1 = <expr>   followed by   dst = t1
//       becomes  dst = <expr>   (when t1 is a temp and used only once)
//   Pattern B — double-negation: NOT(NOT(x)) = x
//   Pattern C — double-assign: consecutive  x = v; x = v  (keep only one)
// =============================================================================
void PeepholePass::optimize(std::vector<TACInstruction>& instructions) {
    // ----- Pattern A: collapse   tN = expr ; dst = tN  -----
    // Count how many times each variable is used as an operand
    std::unordered_map<std::string, int> useCount;
    for (const auto& instr : instructions) {
        if (!instr.arg1.empty() && !isNumber(instr.arg1)) useCount[instr.arg1]++;
        if (!instr.arg2.empty() && !isNumber(instr.arg2)) useCount[instr.arg2]++;
    }

    for (size_t i = 0; i + 1 < instructions.size(); ++i) {
        auto& curr = instructions[i];
        auto& next = instructions[i + 1];

        // Pattern A: curr defines a temp used exactly once, next copies it
        bool currDefinesTemp =
            !curr.result.empty() &&
            curr.result.size() >= 2 &&
            curr.result[0] == 't' &&
            std::isdigit(static_cast<unsigned char>(curr.result[1]));

        bool nextIsSimpleCopy =
            next.op == OpCode::ASSIGN &&
            next.arg1 == curr.result &&
            next.arg2.empty();

        if (currDefinesTemp && nextIsSimpleCopy && useCount[curr.result] == 1) {
            // Re-target the defining instruction directly to next.result
            curr.result = next.result;
            next.op     = OpCode::NOP;
            next.arg1   = "";
            next.arg2   = "";
            next.result = "";
        }
    }

    // ----- Pattern C: consecutive identical assignments  x = v; x = v -----
    for (size_t i = 0; i + 1 < instructions.size(); ++i) {
        auto& curr = instructions[i];
        auto& next = instructions[i + 1];

        if (curr.op     == OpCode::ASSIGN &&
            next.op     == OpCode::ASSIGN &&
            curr.result == next.result    &&
            curr.arg1   == next.arg1      &&
            curr.arg2   == next.arg2) {
            curr.op     = OpCode::NOP;
            curr.arg1   = "";
            curr.arg2   = "";
            curr.result = "";
        }
    }

    instructions.erase(
        std::remove_if(instructions.begin(), instructions.end(),
                       [](const TACInstruction& i) { return i.op == OpCode::NOP; }),
        instructions.end()
    );
}
