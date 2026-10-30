/**
 * ==============================================================================
 * File: src/cfg.h
 * Purpose: Control Flow Graph (CFG), Basic Block Partitioning, and Dominator Trees.
 *
 * Role in Compiler:
 *  - Partitions linear Three-Address Code (TAC) into maximal basic blocks.
 *  - Establishes directed control-flow edges (fallthrough, conditional/unconditional jumps).
 *  - Calculates Dominator Sets (DOM) and Immediate Dominators (IDOM) via iterative dataflow.
 *  - Detects Natural Loops via back-edge identification for loop optimizations (LICM).
 * ==============================================================================
 */

#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <iostream>
#include "tac.h"

struct BasicBlock {
    int id = 0;
    std::string label; // Empty if no leading label
    std::vector<TACInstruction> instructions;
    std::vector<int> predecessors;
    std::vector<int> successors;

    // Dataflow sets
    std::unordered_set<std::string> defSet;
    std::unordered_set<std::string> useSet;
    std::unordered_set<std::string> liveIn;
    std::unordered_set<std::string> liveOut;

    // Dominance
    std::unordered_set<int> dominators;
    int idom = -1; // Immediate dominator
};

struct NaturalLoop {
    int header = -1;                     // Header block ID (dominator target of back-edge)
    int latch = -1;                      // Latch block ID (contains jump back to header)
    std::unordered_set<int> bodyBlocks;  // All block IDs participating in the loop
};

class ControlFlowGraph {
private:
    std::vector<BasicBlock> blocks;
    std::unordered_map<std::string, int> labelToBlock;

    void computeDefUse();

public:
    ControlFlowGraph() = default;

    // Construction from linear TAC instruction stream
    void buildFromTAC(const std::vector<TACInstruction>& instructions);

    // Dominance Analysis
    void computeDominators();

    // Natural Loop Detection
    std::vector<NaturalLoop> detectNaturalLoops() const;

    // Linearization back to TAC
    std::vector<TACInstruction> linearize() const;

    // Getters
    const std::vector<BasicBlock>& getBlocks() const { return blocks; }
    std::vector<BasicBlock>& getBlocks() { return blocks; }
    int size() const { return static_cast<int>(blocks.size()); }

    // Visualization and Reporting
    void printCFG() const;
    std::string toDot() const;
};
