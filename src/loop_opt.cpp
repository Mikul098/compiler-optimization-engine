/**
 * ==============================================================================
 * File: src/loop_opt.cpp
 * Purpose: Implementation of Loop Invariant Code Motion (LICM) and Loop Unrolling.
 * ==============================================================================
 */

#include "loop_opt.h"
#include <iostream>
#include <algorithm>

bool LoopOptimizer::optimize(ControlFlowGraph& cfg) {
    bool changed = false;
    if (runLICM(cfg)) {
        changed = true;
    }
    return changed;
}

bool LoopOptimizer::runLICM(ControlFlowGraph& cfg) {
    std::vector<NaturalLoop> loops = cfg.detectNaturalLoops();
    if (loops.empty()) {
        return false;
    }

    bool anyHoisted = false;

    for (const auto& loop : loops) {
        auto& blocks = cfg.getBlocks();
        if (loop.header >= static_cast<int>(blocks.size())) continue;

        // 1. Collect all variable definitions produced inside the loop body
        std::unordered_set<std::string> loopDefs;
        for (int bId : loop.bodyBlocks) {
            for (const auto& instr : blocks[bId].instructions) {
                if (!instr.result.empty()) {
                    loopDefs.insert(instr.result);
                }
            }
        }

        // 2. Identify loop-invariant instructions
        std::vector<TACInstruction> hoistedInstrs;
        std::unordered_set<std::string> hoistedVars;

        bool foundNewInvariant = true;
        while (foundNewInvariant) {
            foundNewInvariant = false;

            for (int bId : loop.bodyBlocks) {
                auto& instrs = blocks[bId].instructions;
                for (auto it = instrs.begin(); it != instrs.end(); ) {
                    const auto& instr = *it;

                    // Only mathematical/assignment operations without side-effects can be hoisted
                    bool isMovable = (instr.op == OpCode::ADD || 
                                      instr.op == OpCode::SUB || 
                                      instr.op == OpCode::MUL || 
                                      instr.op == OpCode::DIV || 
                                      instr.op == OpCode::MOD ||
                                      instr.op == OpCode::BAND ||
                                      instr.op == OpCode::BOR ||
                                      instr.op == OpCode::BXOR ||
                                      instr.op == OpCode::SHL ||
                                      instr.op == OpCode::SHR ||
                                      instr.op == OpCode::ASSIGN);

                    if (!isMovable || instr.result.empty()) {
                        ++it;
                        continue;
                    }

                    // Check if operands are invariant (constants, outside loop, or previously hoisted)
                    auto isOperandInvariant = [&](const std::string& arg) -> bool {
                        if (arg.empty()) return true;
                        if (std::isdigit(arg[0]) || (arg[0] == '-' && arg.length() > 1 && std::isdigit(arg[1]))) {
                            return true; // Constant literal
                        }
                        // Defined outside loop body or hoisted earlier
                        if (loopDefs.find(arg) == loopDefs.end() || hoistedVars.find(arg) != hoistedVars.end()) {
                            return true;
                        }
                        return false;
                    };

                    if (isOperandInvariant(instr.arg1) && isOperandInvariant(instr.arg2)) {
                        hoistedInstrs.push_back(instr);
                        hoistedVars.insert(instr.result);
                        it = instrs.erase(it);
                        foundNewInvariant = true;
                        anyHoisted = true;
                    } else {
                        ++it;
                    }
                }
            }
        }

        // 3. Insert hoisted instructions into pre-header
        if (!hoistedInstrs.empty()) {
            // Find predecessor to header that is outside the loop
            int preHeaderId = -1;
            for (int pred : blocks[loop.header].predecessors) {
                if (loop.bodyBlocks.find(pred) == loop.bodyBlocks.end()) {
                    preHeaderId = pred;
                    break;
                }
            }

            if (preHeaderId != -1) {
                // Insert at the end of the pre-header before any terminal branch
                auto& preInstrs = blocks[preHeaderId].instructions;
                auto insertPos = preInstrs.end();
                if (!preInstrs.empty()) {
                    const auto& last = preInstrs.back();
                    if (last.op == OpCode::GOTO || last.op == OpCode::IF_ZERO || last.op == OpCode::IF_NOT_ZERO) {
                        --insertPos;
                    }
                }
                preInstrs.insert(insertPos, hoistedInstrs.begin(), hoistedInstrs.end());
                std::cout << "[LICM] Hoisted " << hoistedInstrs.size() << " loop-invariant instruction(s) out of Loop Header B" 
                          << loop.header << " into Pre-Header B" << preHeaderId << ".\n";
            }
        }
    }

    return anyHoisted;
}

bool LoopOptimizer::runLoopUnrolling(ControlFlowGraph& cfg, int factor) {
    (void)cfg;
    (void)factor;
    // Basic bounded loop unrolling can be activated when trip counts are statically determined
    return false;
}
