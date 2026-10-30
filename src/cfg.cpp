/**
 * ==============================================================================
 * File: src/cfg.cpp
 * Purpose: Implementation of Control Flow Graph (CFG) Construction and Analysis.
 * ==============================================================================
 */

#include "cfg.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <queue>

void ControlFlowGraph::buildFromTAC(const std::vector<TACInstruction>& instructions) {
    blocks.clear();
    labelToBlock.clear();

    if (instructions.empty()) {
        return;
    }

    // Step 1: Identify Leaders
    std::vector<bool> isLeader(instructions.size(), false);
    isLeader[0] = true; // First instruction is always a leader

    for (size_t i = 0; i < instructions.size(); ++i) {
        const auto& instr = instructions[i];

        // Target of any branch is a leader
        if (instr.op == OpCode::LABEL) {
            isLeader[i] = true;
        }

        // Instruction immediately following a branch is a leader
        if (instr.op == OpCode::GOTO || instr.op == OpCode::IF_ZERO || instr.op == OpCode::IF_NOT_ZERO) {
            if (i + 1 < instructions.size()) {
                isLeader[i + 1] = true;
            }
        }
    }

    // Step 2: Form Basic Blocks from Leaders
    int currentBlockId = 0;
    BasicBlock currentBlock;
    currentBlock.id = currentBlockId;

    for (size_t i = 0; i < instructions.size(); ++i) {
        if (isLeader[i] && i > 0) {
            if (!currentBlock.instructions.empty()) {
                blocks.push_back(currentBlock);
                currentBlock = BasicBlock();
                currentBlock.id = ++currentBlockId;
            }
        }

        if (instructions[i].op == OpCode::LABEL) {
            currentBlock.label = instructions[i].arg1;
            labelToBlock[instructions[i].arg1] = currentBlock.id;
        }

        currentBlock.instructions.push_back(instructions[i]);
    }

    if (!currentBlock.instructions.empty()) {
        blocks.push_back(currentBlock);
    }

    // Index all labels to block IDs
    for (size_t b = 0; b < blocks.size(); ++b) {
        for (const auto& instr : blocks[b].instructions) {
            if (instr.op == OpCode::LABEL) {
                labelToBlock[instr.arg1] = static_cast<int>(b);
            }
        }
    }

    // Step 3: Establish Directed Edges (Predecessors & Successors)
    for (size_t b = 0; b < blocks.size(); ++b) {
        auto& blk = blocks[b];
        if (blk.instructions.empty()) continue;

        const auto& lastInstr = blk.instructions.back();

        if (lastInstr.op == OpCode::GOTO) {
            auto it = labelToBlock.find(lastInstr.arg1);
            if (it != labelToBlock.end()) {
                int targetId = it->second;
                blk.successors.push_back(targetId);
                blocks[targetId].predecessors.push_back(static_cast<int>(b));
            }
        } else if (lastInstr.op == OpCode::IF_ZERO || lastInstr.op == OpCode::IF_NOT_ZERO) {
            // Taken branch
            auto it = labelToBlock.find(lastInstr.arg2);
            if (it != labelToBlock.end()) {
                int targetId = it->second;
                blk.successors.push_back(targetId);
                blocks[targetId].predecessors.push_back(static_cast<int>(b));
            }
            // Fallthrough branch
            if (b + 1 < blocks.size()) {
                int fallId = static_cast<int>(b + 1);
                blk.successors.push_back(fallId);
                blocks[fallId].predecessors.push_back(static_cast<int>(b));
            }
        } else {
            // Normal sequential fallthrough
            if (b + 1 < blocks.size()) {
                int fallId = static_cast<int>(b + 1);
                blk.successors.push_back(fallId);
                blocks[fallId].predecessors.push_back(static_cast<int>(b));
            }
        }
    }

    computeDefUse();
    computeDominators();
}

void ControlFlowGraph::computeDefUse() {
    for (auto& blk : blocks) {
        blk.defSet.clear();
        blk.useSet.clear();

        for (const auto& instr : blk.instructions) {
            // Check uses (arg1, arg2)
            auto checkUse = [&](const std::string& arg) {
                if (!arg.empty() && !std::isdigit(arg[0]) && arg[0] != '-') {
                    if (blk.defSet.find(arg) == blk.defSet.end()) {
                        blk.useSet.insert(arg);
                    }
                }
            };

            if (instr.op != OpCode::LABEL && instr.op != OpCode::GOTO) {
                checkUse(instr.arg1);
                checkUse(instr.arg2);
            }

            // Check def (result)
            if (!instr.result.empty()) {
                blk.defSet.insert(instr.result);
            }
        }
    }
}

void ControlFlowGraph::computeDominators() {
    if (blocks.empty()) return;

    int n = static_cast<int>(blocks.size());

    // All blocks set for initial intersection
    std::unordered_set<int> allBlocks;
    for (int i = 0; i < n; ++i) {
        allBlocks.insert(i);
    }

    // Dom(B0) = {B0}
    blocks[0].dominators = {0};

    // Dom(Bi) = all blocks for i > 0
    for (int i = 1; i < n; ++i) {
        blocks[i].dominators = allBlocks;
    }

    // Iterative dataflow solver
    bool changed = true;
    while (changed) {
        changed = false;

        for (int i = 1; i < n; ++i) {
            auto& blk = blocks[i];
            if (blk.predecessors.empty()) continue;

            // Intersection of Pred(Bi) dominator sets
            std::unordered_set<int> newDom = blocks[blk.predecessors[0]].dominators;
            for (size_t p = 1; p < blk.predecessors.size(); ++p) {
                int predId = blk.predecessors[p];
                const auto& pDom = blocks[predId].dominators;
                std::unordered_set<int> intersect;
                for (int d : newDom) {
                    if (pDom.find(d) != pDom.end()) {
                        intersect.insert(d);
                    }
                }
                newDom = intersect;
            }

            newDom.insert(i); // {Bi} U intersection

            if (newDom != blk.dominators) {
                blk.dominators = newDom;
                changed = true;
            }
        }
    }

    // Compute immediate dominators (idom)
    for (int i = 1; i < n; ++i) {
        auto& blk = blocks[i];
        for (int d : blk.dominators) {
            if (d == i) continue;
            // d is idom(i) if no other strict dominator of i is strictly dominated by d
            bool isImmediate = true;
            for (int other : blk.dominators) {
                if (other == i || other == d) continue;
                if (blocks[other].dominators.find(d) != blocks[other].dominators.end()) {
                    isImmediate = false;
                    break;
                }
            }
            if (isImmediate) {
                blk.idom = d;
                break;
            }
        }
    }
}

std::vector<NaturalLoop> ControlFlowGraph::detectNaturalLoops() const {
    std::vector<NaturalLoop> loops;

    for (size_t u = 0; u < blocks.size(); ++u) {
        const auto& blk = blocks[u];
        for (int v : blk.successors) {
            // Back-edge u -> v exists if v dominates u
            if (blk.dominators.find(v) != blk.dominators.end()) {
                NaturalLoop loop;
                loop.header = v;
                loop.latch = static_cast<int>(u);
                loop.bodyBlocks.insert(v);
                loop.bodyBlocks.insert(static_cast<int>(u));

                // Traverse backwards from latch u to find all blocks in loop
                std::queue<int> q;
                if (static_cast<int>(u) != v) {
                    q.push(static_cast<int>(u));
                }

                while (!q.empty()) {
                    int curr = q.front();
                    q.pop();

                    for (int pred : blocks[curr].predecessors) {
                        if (loop.bodyBlocks.find(pred) == loop.bodyBlocks.end()) {
                            loop.bodyBlocks.insert(pred);
                            q.push(pred);
                        }
                    }
                }

                loops.push_back(loop);
            }
        }
    }

    return loops;
}

std::vector<TACInstruction> ControlFlowGraph::linearize() const {
    std::vector<TACInstruction> out;
    for (const auto& blk : blocks) {
        for (const auto& instr : blk.instructions) {
            out.push_back(instr);
        }
    }
    return out;
}

void ControlFlowGraph::printCFG() const {
    std::cout << "=== CONTROL FLOW GRAPH (" << blocks.size() << " Basic Blocks) ===\n";
    for (const auto& blk : blocks) {
        std::cout << "[Block B" << blk.id << "]";
        if (!blk.label.empty()) std::cout << " (Label: " << blk.label << ")";
        std::cout << "\n  Preds: ";
        for (int p : blk.predecessors) std::cout << "B" << p << " ";
        std::cout << "\n  Succs: ";
        for (int s : blk.successors) std::cout << "B" << s << " ";
        std::cout << "\n  Dominators: ";
        for (int d : blk.dominators) std::cout << "B" << d << " ";
        if (blk.idom != -1) std::cout << " | IDOM: B" << blk.idom;
        std::cout << "\n  Instructions (" << blk.instructions.size() << "):\n";
        for (const auto& instr : blk.instructions) {
            std::cout << "    " << instr.toString() << "\n";
        }
        std::cout << "\n";
    }
}

std::string ControlFlowGraph::toDot() const {
    std::stringstream ss;
    ss << "digraph CFG {\n";
    ss << "  node [shape=record, fontname=\"Consolas\"];\n";

    for (const auto& blk : blocks) {
        ss << "  B" << blk.id << " [label=\"{B" << blk.id;
        if (!blk.label.empty()) ss << " (" << blk.label << ")";
        ss << " | ";
        for (size_t i = 0; i < blk.instructions.size(); ++i) {
            std::string s = blk.instructions[i].toString();
            // Escape special chars
            for (char c : s) {
                if (c == '<' || c == '>' || c == '{' || c == '}' || c == '|' || c == '\"') {
                    ss << '\\';
                }
                ss << c;
            }
            if (i + 1 < blk.instructions.size()) ss << "\\l";
        }
        ss << "\\l}\"];\n";

        for (int s : blk.successors) {
            ss << "  B" << blk.id << " -> B" << s << ";\n";
        }
    }

    ss << "}\n";
    return ss.str();
}
