/**
 * ==============================================================================
 * File: src/regalloc.h
 * Purpose: Chaitin-Briggs Graph-Coloring Register Allocator with Stack Spilling.
 *
 * Role in Compiler:
 *  - Solves the NP-complete register allocation problem using Kempe/Chaitin heuristics.
 *  - Maps unbounded virtual TAC temporaries to finite x86-64 hardware registers.
 *  - Automatically spills variables to the stack frame when chromatic number exceeds K.
 * ==============================================================================
 */

#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "liveness.h"

struct RegAllocResult {
    std::unordered_map<std::string, std::string> registerMap; // var -> physical register (e.g. "rbx")
    std::unordered_map<std::string, int> stackSpills;         // var -> stack offset (e.g. -24)
    int spillCount = 0;
    int stackFrameBytes = 0;
    int kRegisters = 0;
    bool successWithoutSpills = true;
};

class ChaitinRegisterAllocator {
private:
    std::vector<std::string> physicalRegisters;
    int K;

public:
    // Default pool uses standard non-reserved x86-64 general purpose registers
    explicit ChaitinRegisterAllocator(int k = 6);

    void setRegisterPool(const std::vector<std::string>& regs);

    RegAllocResult allocate(const InterferenceGraph& originalGraph);

    void printAllocation(const RegAllocResult& result) const;
};
